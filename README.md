# shapes-destroyer-c

This is just a little study project made with C and SDL3.
Hopefully I'll limit myself only to SDL3 as a library, I know is quite big but I never wrote anything in C before so is a trade of I'm willing to make before going deeper.


## Objective

Make a tuned down version of Geometry Wars and use only SDL3 as an external library.

## Why?

I want to limit only to SDL3 as an external library in order to understand how to make stuff in C.

I would like to implement myself everything I could need for the game. For example an ECS Framework, a basic one obviously, but I want to try.
The idea is to face problems and solve them in C and learn from the experience.

## TODOs

- [ ] render the player in a different way
- [ ] make the player move
- [ ] render the inside of the shape in a different color (is it difficult?)
- [ ] move the systems in another file
- [ ] add a basic debug UI, don't go crazy, this is an excercise you can use Nuklear
- [ ] try the trick of a struct with pointers to the function, in order to have simpler names for the functions
  - is that the C way to do things? figure out, and even if it's not try it anyway :)
- [ ] add a Transform component that contains the position and also the angle of rotation
- [ ] add a system that constantly rotates the rendered shapes
- [ ] find a way to add Tags to the entities and filter the entitis by that tag
  - So you can have the Player tag e properly find the player, or the Enemies or the Bullets etc.
- [ ] use the new shiny math functions you created in the currently implemented systems
- [ ] add a way to enable/disable systems
- [ ] add the scene concept to the game
- [ ] add a intro screen
- [ ] add a pause menu scene with simple controls, just to implement the thing and see how it works
- [ ] sounds, I know you hate it but eventually you have to do it
- [ ] add controller support
- [ ] shaders?
