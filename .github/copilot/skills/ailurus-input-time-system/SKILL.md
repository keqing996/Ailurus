# Ailurus Input & Time Systems

## Scope
Keyboard/mouse input handling via SDL3, button state tracking, input callbacks, frame timing, and elapsed time measurement.

## Key Files
- `include/Ailurus/Systems/InputSystem/InputSystem.h` — Input management
- `include/Ailurus/Systems/InputSystem/InputEnum.h` — ButtonType enum (~120+ values)
- `src/Systems/InputSystem/InputSystem.cpp` — SDL3 event mapping
- `src/Systems/InputSystem/InputEnum.cpp` — ButtonType name strings
- `include/Ailurus/Systems/TimeSystem/TimeSystem.h` — Time tracking
- `src/Systems/TimeSystem/TimeSystem.cpp` — Frame/delta/elapsed implementation

## InputSystem

### ButtonType Enum (~120+ values)
```
Mouse: MouseLeft, MouseMiddle, MouseRight, MouseXButton1, MouseXButton2
Keyboard Letters: KeyboardA–KeyboardZ
Keyboard Numbers: KeyboardNum0–KeyboardNum9
Function Keys: KeyboardF1–KeyboardF24
Arrows: KeyboardArrowUp/Down/Left/Right
NumPad: KeyboardNumPad0–9, +, -, *, /, ., Lock, Enter
Control: Escape, Tab, Capital, LShift/LCtrl/LSuper/LAlt, Space, RShift/RCtrl/RSuper/RAlt, Enter, Backspace
Edit: Insert, Delete, PageUp/Down, Home, End
Signs: Comma, Period, Slash, Semicolon, Apostrophe, Brackets, Backslash, GraveAccent, Minus, Equal
Media: VolumeMute/Up/Down, PlayPause, Stop, Prev, Next
Special: Unknown, Count
```

`ButtonTypeUtility::GetName(ButtonType)` returns human-readable C-string.

### InputSystem Class
Private constructor (friend of Application). NonCopyable, NonMovable.

**Members:**
- `Vector2f _lastMousePos, _mousePos` — Previous/current mouse position
- `Vector2f _mouseWheel` — Accumulated wheel delta per frame
- `bool _enableAutoRepeat` — Default true
- `unordered_set<ButtonType> _pressedButton` — Currently pressed buttons

**State Queries:**
- `IsButtonPressed(ButtonType)` → bool — O(1) lookup via unordered_set
- `GetMousePosition()` → Vector2f
- `GetMouseWheel()` → Vector2f

**Callbacks:**
- `SetCallbackOnMouseMove(fn(Vector2f oldPos, Vector2f newPos))`
- `SetCallbackOnMouseWheel(fn(Vector2f delta))`
- `SetCallbackOnButtonPressed(fn(ButtonType))`
- `SetCallbackOnButtonReleased(fn(ButtonType))`

**Per-Frame Lifecycle (called by Application):**
1. `BeforeEventLoop()` — Cache last mouse pos, reset wheel delta
2. `HandleEvent(sdlWindow, sdlEvent)` — Maps SDL3 events to ButtonType
3. `AfterEventLoop()` — Emit accumulated callbacks

**SDL3 Event Mapping:**
- `SDL_EVENT_MOUSE_MOTION` → mouse position update
- `SDL_EVENT_MOUSE_WHEEL` → wheel delta accumulation
- `SDL_EVENT_MOUSE_BUTTON_DOWN/UP` → mouse button state
- `SDL_EVENT_KEY_DOWN/UP` → keyboard state (with auto-repeat filtering)

## TimeSystem

Private constructor (friend of Application). NonCopyable, NonMovable.

**Members:**
- `uint64_t _frameCount` — 0-based frame counter
- `Timer<> _frameTimer` — Microsecond-precision frame timer
- `double _deltaTime` — Seconds since last Update()
- `Timer<TimePrecision::Nanoseconds> _elapsedTimer` — Total elapsed timer

**API:**
- `FrameCount()` → uint64_t — Current frame number
- `DeltaTime()` → double — Seconds since last frame
- `GetElapsedTime()` → double — Total seconds since system creation

**Per-Frame:** `Update()` increments frame count, reads frame timer via `GetIntervalAndSetNow()`, resets for next frame.
