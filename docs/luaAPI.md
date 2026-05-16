# Flux documentation - Lua API

  

 Welcome to the manual of scripting in Flux, this will be your scripting reference manual (if you don't know what you're doing). You can use these built-in functions to control your game!

---

### Global functions

`onStart()`

* This is called when the game has started. (it's pretty simple and self-explanatory,)
##### This is pretty useful for loading by the way.

```lua
function onStart()
	print("Hello World!")
	print("Loading data...")
	Load()
	print("Load Successfully!")
end
```
 `onUpdate()`

* This function is called on every frame of the game, pretty useful if you can handle it right.

```lua
local count = 0
function onUpdate()
	if count > 10 then
		count = count + 1
		print(tostring(count))
	end
end
```

`onEnd()`

* This function is called right before the game ends
##### psst, it's pretty useful if you want to make a saving system ;)

```lua
function onEnd()
	print("Saving...")
	Save()
	print("Saved successfully!")
end
```

---
### Input system
The input system usually starts with a `Input.` syntax.

`Input.`
