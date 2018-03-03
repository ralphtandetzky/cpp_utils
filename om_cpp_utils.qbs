import qbs

StaticLibrary
{
    name: "om_cpp_utils"
    Depends { name: "cpp" }

    Export
    {
        Depends { name: "cpp" }
        cpp.cxxLanguageVersion: "c++17"
        cpp.defines: [
            "QT_DISABLE_DEPRECATED_BEFORE=0x050800",
            "BOOST_NO_AUTO_PTR",
            //"BOOST_SYSTEM_NO_DEPRECATED",
            "OM_STATIC_BUILD"
        ]
        cpp.windowsApiCharacterSet: "mbcs"
    }

    property bool link_statically: true
    cpp.defines: [
        "QT_DISABLE_DEPRECATED_BEFORE=0x050800",
        "BOOST_NO_AUTO_PTR",
        //"BOOST_SYSTEM_NO_DEPRECATED",
        "OM_STATIC_BUILD"
    ]
    cpp.windowsApiCharacterSet: "mbcs"

    files: [
        "algorithm.hpp",
        "array_arith.hpp",
        "c++17_features.hpp",
        "concurrent.hpp",
        "concurrent_queue.hpp",
        "cow_ptr.hpp",
        "dependency_thread_pool.hpp",
        "exception.hpp",
        "exception_handling.hpp",
        "filters.hpp",
        "functors.hpp",
        "functors_fwd.hpp",
        "fwd.hpp",
        "geometry.hpp",
        "hexdump.hpp",
        "ignore.hpp",
        "int_traits.hpp",
        "math_helpers.hpp",
        "memory_helpers.hpp",
        "meta_functor_binder.hpp",
        "meta_programming.hpp",
        "minimize_differential_evolution.hpp",
        "minimize_nelder_mead.hpp",
        "monitor.hpp",
        "pimpl_ptr.hpp",
        "polynomials.hpp",
        "progress.hpp",
        "ranges.hpp",
        "rank.hpp",
        "rational.hpp",
        "region_allocator.hpp",
        "scope_guard.hpp",
        "slice.hpp",
        "string_helpers.hpp",
        "swap.hpp",
        "task_blocker.hpp",
        "task_queue.hpp",
        "task_queue_thread.hpp",
        "task_queue_thread_pool.hpp",
        "units.hpp",
        "updater.hpp",
        "vector_arith.hpp",
        "visitor.hpp",
    ]
}
