# sdl-fixes
Fix SDL Linux evdev leaving keyboard dead after nonclean application exit.

In SDL release 2.0.6, when Linux evdev keyboard support has been moved to a separate source file, a feature was added to disable normal keyboard event processing, to prevent "spilling" keystrokes to background virtual console.

This feature has one unpleasant side effect: if application fails to call `SDL_Exit` before termination or crashes with fatal signal, console is left in unusable state with keyboard not working and no possibility to switch virtual console. If user has a chance, he can login remotely and restore keyboard with `kbd_mode`, otherwise the only option is to reboot the machine.

This patch fixes that problem by intercepting fatal signals (with `sigaction`) and process termination (with `atexit`), to restore keyboard state, if it wasn't properly restored with `SDL_Exit`.

The function registered with `atexit` also restores original signal handlers, to prevent leaving invalid handlers after SDL library is unloaded, if it was loaded dynamically with `dlopen`.

No signal handlers or `atexit` function are installed if SDL boolean hint `SDL_HINT_NO_SIGNAL_HANDLERS` is `SDL_TRUE`.

Additionally, if environment variable `SDL_INPUT_LINUX_KEEP_KBD` exists, keyboard initialization function completely skips disabling keyboard. This can be useful for e.g. debugging.
