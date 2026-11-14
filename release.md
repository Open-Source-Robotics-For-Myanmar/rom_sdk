# How to release 

## check current release

```bash
version=$(awk '/^[[:space:]]*project\(rom([[:space:]]|$)/ {p=1} p && $1 == "VERSION" {print $2; exit}' CMakeLists.txt)
echo $version
```
this may be 0.0.0

## edit release+1 version in CMakeLists.txt
```make
# edit
project(rom
  VERSION 0.0.1
  DESCRIPTION "ROM Robotics C++17 SDK"
  LANGUAGES CXX)
```

## tag for workflow
```bash
version=$(awk '/^[[:space:]]*project\(rom([[:space:]]|$)/ {p=1} p && $1 == "VERSION" {print $2; exit}' CMakeLists.txt)
echo $version

git tag release/v$version
```