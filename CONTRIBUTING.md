# Flux Contributing

Flux is licensed in GPL v3, that means you have to make your contribution open sourced if you're gonna share your version of Flux to someone else. 

Flux is written in C++ from scratch, no frameworks like Qt and Electron, just CMake, Vcpkg, and a couple of libraries.

If you're reading this, you either are a nerd, or you just don't understand what Idkthisguy's code means. Don't worry this document is made by Idkthisguy (obviously, who else would it be?), this document is gonna teach you what you need to know about the source code.


## Engine

### Viewport & Rendering 
### `Involved classes & scripts: OpenGL Manager, Viewport, 3DRenderer, Camera, Window and Model`

Well, this was one of the first things that was made in Flux, it's also one of the most important and one the most hardest things to make.

#### Viewport
Viewport is simple, it's the monitor to whatever you're making.