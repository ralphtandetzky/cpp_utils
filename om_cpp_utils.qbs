import qbs

StaticLibrary
{
    name: "om_cpp_utils"
    Depends { name: "cpp" }

    Export
    {
        Depends { name: "cpp" }
        cpp.cxxLanguageVersion: "c++14"
        cpp.defines: [ "OM_STATIC_BUILD" ]
        cpp.windowsApiCharacterSet: "unicode"
    }

    files: [
        "algorithm.hpp",
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
        "json.hpp",
        "json_fwd.hpp",
        "math_helpers.hpp",
        "memory_helpers.hpp",
        "meta_functor_binder.hpp",
        "meta_programming.hpp",
        "monitor.hpp",
        "pimpl_ptr.hpp",
        "polynomials.hpp",
        "ranges.hpp",
        "rank.hpp",
        "region_allocator.hpp",
        "scope_guard.hpp",
        "string_helpers.hpp",
        "swap.hpp",
        "task_queue.hpp",
        "task_queue_thread.hpp",
        "task_queue_thread_pool.hpp",
        "units.hpp",
        "updater.hpp",
        "visitor.hpp",
    ]
}
