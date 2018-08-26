#ifndef NGINCC_PLUGIN_HXX
#define NGINCC_PLUGIN_HXX

#include <any>
#include <functional>
#include <unordered_map>
#include <tuple>

#include "module.hxx"         // defines module

namespace ngincc {
    namespace core {

        /* deleteme struct plugin_callback {
            int callback(string& input, string&output)
        };*/

        class plugin_manager : public ngincc::core::module {
        public:
            plugin_manager();
            ~plugin_manager();
            //! returns plugin_id
            template<typename ...Targs>
            int plug_add(const std::string&& plugin_space, const std::string&& desc, const std::function<int(Targs...)>&& plug) {
                return plug_add_helper(move(plugin_space),move(desc),move(plug));
            }
            int plug_remove(int plugin_id);

            //! \@{ look-up plugin
            //! \brief Get a plugin by the plugin_id
            //template <typename T>
            //typename T& plug_get(int plugin_id);
            //! \brief Get a plugin by the plugin_space
            //iterator find_plug(const std::string &plugin_space) {
            //    return plugs.find(plugin_space);
            //}
            template <typename ...Targs>
            int plug_call(const std::string& plugin_space, std::tuple<Targs...>&& args) {
                const unsigned long long hcode = hash_func(plugin_space);
                // auto it = plugs.find(hcode);
                auto&& range = plugs.equal_range(hcode);
                for(auto&& it = range.first; it != range.second; it++) {
                    if(0 == plugin_space.compare(std::get<0>(it->second))) {
                        //std::apply<std::function<int(Targs...)>,std::tuple<Targs...> >(std::any_cast<std::function<int(Targs...)>>(std::get<2>(it->second)), args);
                        std::apply(std::any_cast<std::function<int(Targs...)>>(std::get<2>(it->second)), args);
                    }
                }
                // TODO return a vector of all return values
                return 0;
            }
            //! \@}
            auto begin() const noexcept {
                return plugs.cbegin();
            }
            auto end() const noexcept {
                return plugs.cend();
            }
        private:
            //! TODO use hash-table/numbered,linked-list queue/radix-tree here.
            std::unordered_multimap<unsigned long long,std::tuple<std::string, std::string, std::any > > plugs;
            std::hash<std::string> hash_func;
            int plug_add_helper(const std::string&& plugin_space, const std::string&& desc, const std::any&& plug);
        };
    };
}


#endif // NGINCC_PLUGIN_HXX
