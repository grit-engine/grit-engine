#ifndef lua_userdata_dependency_tracker_funcs_h
#define lua_userdata_dependency_tracker_funcs_h

template <typename T>
static void vec_nullify_remove (std::vector<void**>& v, T **ud)
{
        void *const* ptr = (void *const*) ud; //thankyou c++
        std::vector<void**>::iterator i = std::find(v.begin(),v.end(),ptr);
        if (i==v.end()) {
                CERR<<"gc'ing: Spurious userdata!"<<std::endl;
                app_fatal();
        }
        **i = NULL;
        v.erase(i);
}

static void vec_nullify_remove_all (std::vector<void**>& v)
{
        for (std::vector<void**>::iterator i = v.begin(); i!=v.end() ; i++) {
                **i = NULL;
        }
        v.clear();
}

template <typename T, typename T2>
static void map_remove_only (std::map<T,T2>& m, T k)
{
        typename std::map<T,T2>::iterator i = m.find(k);
        if (i==m.end()) {
                CERR<<"gc'ing: Map does not contain item!"<<std::endl;
                app_fatal();
        }
        m.erase(i);
}

template <typename T>
static void map_nullify_remove (std::map<T,std::vector<void**> >&m, T k)
{
        vec_nullify_remove_all (m[k]);
        map_remove_only(m,k);
}

template <typename T>
static void map_nullify_remove_all (std::map<T*,std::vector<void**> >& map)
{
        typedef typename std::map<T*,std::vector<void**> >::iterator iter;
        for (iter i = map.begin(); i!=map.end() ; i++) {
                vec_nullify_remove_all(i->second);
        }
        map.clear();
}

#endif

// vim: shiftwidth=8:tabstop=8:expandtab
