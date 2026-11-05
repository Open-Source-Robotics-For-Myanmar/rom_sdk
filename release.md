# How to release 

```bash
version=$(awk '/^[[:space:]]*project\(rom([[:space:]]|$)/ {p=1} p && $1 == "VERSION" {print $2; exit}' CMakeLists.txt)
git tag release/v$version
```