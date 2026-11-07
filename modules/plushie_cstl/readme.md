# Plushie CSTL 🧸🐳

*My little "C Standard Library" which is a collection of **header only** implementations for things that I think might be commonly reusable as a small component, or something that I do often reuse*

## Local Dev Install

CMake uses the `find_package` to add the `Plushie::CStl` interface target. If you're a consumer of the package, then that's all you'll need. However you might be working on a different project, and you want to develop a `CStl`  component then you can use the *"Local dev install"*

This places the `PlushieCStlConfig.cmake` in this source directory and that will allow you to use this source tree and `find_package(PlushieCStl)` 

```bash
cmake -S . -B build -DPLUSHIE_CSTL_INSTALL_FOR_LOCAL_DEV=ON
cmake --build build
cmake --install build --prefix .

export PlushieCStl_DIR="$pwd"
```

## Testing

The CSTL repo itself should be small in file size, so that a user can download it easily. Obviously we need testing, so we put that in another repo : [GitHub - PnyaaPlushie/plushie_cstl_tests: Test cases for CSTL](https://github.com/PnyaaPlushie/plushie_cstl_tests) This way we can keep our redistributables small, and still validate the actual repo itself

## Examples

* Allocator - A lot of subprojects need an allocation strategy, I prefer to pass allocators as a parameter. It means we're not implicitly calling stdlib functions, and a library user can replace the allocation strategy if they're not happy with my implementation

* Elf file format - I needed to decode elf file formats for my riscv emulator, but I think it makes sense to make that reusable.
