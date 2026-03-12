# Ailurus Utility & OS Modules

## Scope
Cross-platform utilities (color, string, file I/O, logging, enum reflection, timers, image loading) and OS abstractions (virtual memory, paths, processes, system info).

## Key Files

### Utility
- `include/Ailurus/Utility/Color.h` — RGBA color (uint8 or float)
- `include/Ailurus/Utility/CommandLine.h` — Argument parser
- `include/Ailurus/Utility/EnumReflection.h` — Compile-time enum ↔ string via REFLECTION_ENUM macro
- `include/Ailurus/Utility/File.h` — File I/O and CSV parsing
- `include/Ailurus/Utility/Image.h` — Image loading (stb_image)
- `include/Ailurus/Utility/Logger.h` — Logging wrapper (spdlog)
- `include/Ailurus/Utility/NonCopyable.h` — Deleted copy ops
- `include/Ailurus/Utility/NonMovable.h` — Deleted move ops
- `include/Ailurus/Utility/ScopeGuard.h` — RAII cleanup
- `include/Ailurus/Utility/Singleton.h` — (Empty placeholder)
- `include/Ailurus/Utility/SpinPause.h` — CPU pause instruction
- `include/Ailurus/Utility/String.h` — Split, join, replace, trim, wide string conversion
- `include/Ailurus/Utility/Timer.h` — High-precision timer (ns/us/ms/s)
- `src/Utility/` — Implementations

### OS
- `include/Ailurus/OS/Memory.h` — Virtual memory reserve/commit/release
- `include/Ailurus/OS/Path.h` — Platform-specific resource root, path resolution
- `include/Ailurus/OS/Process.h` — Subprocess creation with stdin/stdout/stderr pipes
- `include/Ailurus/OS/System.h` — Env vars, directories, machine/user name
- `src/OS/Posix/` — Linux/macOS implementations
- `src/OS/Apple/` — macOS bundle-aware path resolution
- `src/OS/Windows/` — Windows implementations + Console, FileDialog, DPI utilities
- `src/OS/Android/` — Android logcat and asset paths

## Utility Classes

### Color
Members: `uint8_t r, g, b, a`.
Constructors: default (black opaque), from packed `uint32_t`, from RGB/RGBA uint8, from RGBA float (0–1).
Methods: `Pack()` → uint32_t. Operators: `==`, `!=`.

### CommandLine
`AddOption(fullName, [shortName], desc)` → register options.
`Parse(argc, argv)` → parse command line.
`operator[](fullName)` → `Result*` (vector of string values, nullptr if not found).
`GenerateHelpMsg()` → formatted help string.

### EnumReflection<EnumType>
Template class requiring `is_enum_v<EnumType>`.
- `Size()` → int (count of values)
- `ToString(EnumType)` → `const string&`
- `FromString(string)` → EnumType (throws on invalid)
- `TryFromString(string, EnumType*)` → bool
- `GetNameArray()`, `GetEnumArray()` → arrays

**REFLECTION_ENUM(Name, ...)** macro: Creates enum class + specialization in one step.

### File (static utility)
- `LoadBinary(path)` → `optional<vector<char>>`
- `LoadText(path)` → `optional<string>`
- `EnsureDirectoryExist(path)`
- `GetFileName(path)`, `GetFileNameWithoutExtension(path)`, `GetFileExtension(path)`
- `File::CSV::SplitCsvLine(str)` → `vector<string>` (handles quoted fields)

### Image
Members: `vector<uint8_t> _data`, `uint _width`, `uint _height` (RGBA 4 bytes/pixel).
Constructors: from dimensions + fill color, from raw data, from file path (stb_image), from memory buffer.
Methods: `GetPixelSize()`, `GetPixel(x,y)`, `SetPixel(x,y,color)`, `VerticalFlip()`, `GetBytesData()`, `GetPixelsData()`.

### Logger (static utility, wraps spdlog)
Levels: `Info`, `Warning`, `Error`. Filter via `SetFilterLevel(level)`.
Methods: `LogInfo/LogWarn/LogError(msg)`, template variants with `std::format` args.

### NonCopyable / NonMovable
Base classes with deleted copy/move constructors and assignment operators.

### ScopeGuard : NonCopyable
Constructor takes callable, destructor executes it. Non-movable.

### SpinPause()
Free function: x86 `_mm_pause()`, ARM64 `yield`, ARM32 `nop`.

### String (static utility)
- `Split(str, delimiter)` → `vector<string>`, `SplitView()` → `vector<string_view>`
- `Join(vec, delimiter)` → string
- `Replace(str, from, to)` — in-place and copy variants
- `TrimStart/TrimEnd/Trim(str)` — whitespace removal
- `WideStringToString(wstr)`, `StringToWideString(str)` — UTF-8 ↔ wide

### Timer<TimePrecision T = Milliseconds>
Precisions: `Nanoseconds`, `Microseconds`, `Milliseconds`, `Seconds`.
Methods: `SetNow()`, `GetInterval()` → int64_t, `GetIntervalAndSetNow()` → int64_t.

## OS Classes

### Memory (static utility)
- `VirtualReserve(addr, size)` → `void*` — Reserve pages (POSIX: `mmap PROT_NONE`, Win: `MEM_RESERVE`)
- `VirtualCommit(addr, size)` → bool — Commit pages (POSIX: `mprotect RW`, Win: `MEM_COMMIT`)
- `VirtualRelease(addr, size)` — Free pages (POSIX: `munmap`, Win: `VirtualFree`)
- `CurrentPageSize()` → size_t

### Path (static utility)
- `GetResourceRootPath()` → string — macOS: bundle Resources dir; others: executable dir
- `ResolvePath(path)` → string — Resolve relative to resource root
- `IsAbsolutePath(path)` → bool

### Process
Members: `Handle handle`, `Pipe stdinPipe/stdoutPipe/stderrPipe`.
- `Create(exeName, argv)` → `optional<Process>` — Fork+exec (POSIX) or CreateProcess (Win)
- `IsRunning()`, `WaitFinish()` → exit code
- `WriteStdin(buf, size)`, `ReadStdout(buf, max)`, `ReadStderr(buf, max)`
- Static: `GetCurrentProcessId()`, `GetProcessName(pid)`

### System (static utility)
- `GetMachineName()`, `GetCurrentUserName()`
- `Get/SetEnvironmentVariable(key, [value])`
- `GetHomeDirectory()`, `GetCurrentDirectory()`, `SetCurrentDirectory(path)`
- `GetExecutableDirectory()`, `GetTempDirectory()`
- `GetLastError()` → uint32_t

### Windows-Only
- **Console**: `CreateConsole()`, `AttachConsole()`, color output, progress bar
- **Dialog**: `OpenFile()`, `SaveFile()`, `OpenDirectory()` — COM IFileDialog wrappers
- **NativeWindowUtility**: `FixProcessDpi()` — DPI awareness setup

### Android-Only
- **Android**: `GetLogFunction()`, `AndroidLogCat(level, tag, msg)` — logcat integration
