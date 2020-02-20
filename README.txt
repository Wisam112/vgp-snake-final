####################################################snake game -3D animations #################################
#1 how to the run the program.
#2 how to compile the code from start to end .( if you want to )
#3 structure and game logic.
#4 technical elements and issues we overcome. 
#5 resources used in building the game .


    #     #            ####           #            #              ####
    #     #            #              #            #             #    #
    #######            ####           #            #             #    #
    #     #            #              #            #             #    #
    #     #            ####           #####        ####           ####


#################################### how to the run the program########################

##place the sandbox.exec file in same directory as the dll files.

##place audio and textures (audio ,text)  folders in same directory as sandbox.exec.


#################################### how to compile the code from start to end#################

##compile  EngineForAnimationCourse-master with cmakeGui  output compiled code to  new project directory.

## go to the new project directory and start the VS solution sandbox (tutorial/sandox).

##link the audio library to the solution via proprieties of sandbox in VS :
###add EngineForAnimationCourse-masterinclude to linking directories in proprieties. 
###add windows-vs64 folder to c/c++ linking directory in proprieties.
###add irrKLang.lib to c/c++ input in proprieties.

## build the solution via VS and run sandbox.exec from project directory.

#################################### structure and game logic ##############

## game environment is 3D space with virtual walls , snake shaped object built from 11 y-cylinders , at least 10 bouncing spheres in space .

## the animations are spheres moving in space in specific velocity  and they are bouncing of each other and walls ,the snake object is trying to reach sphere and have 10 
 free angles were its can rotate in space and its also move in direction of specific sphere.

## there is background sound in game , sound will be generated when game end ,  snake reach sphere also be generated when sphere bounce off another or off walls.

## snake will try to reach sphere when player click on that sphere after game start with the mouse .

## player target is in every level to collect all 10 sphere using the snake as fast  as possible  and avoid clicking on explosive spheres , a given time will  start to tick down when player start each level and stop when  player collect all spheres.

## after each level player have the choice to move to next level or try improve his score and restart the game  , with each level forward in game  sphere moving speed 
 will increase as well as snake speed and the number of explosive sphere will increase .

## there is no level limits , players will try to complete as many level as they can .

## UI is in separate screen , player given the choice to mute music or game sounds and restart game and also we save all time best score and show it on screen ,  so he can beat it.


####################################  technical elements and issues we overcome ################
## first we had to create virtual 3D space to give us the walls and vibe of 3D movement so we used huge cube and built the game in it and used its corners for 3D feel
 and we inverted its normals so we can put color  texture on it in the inside .

## we used 11 y-cylinders to represent the snake , each cylinder have its proprieties , we scaled the cylinders inside each other and   put snake texture on top them to 
 emulate snake look.

## for sphere we used glassy texture to reflect color of the sphere and made rotate around itself in angle determined by its speed in addition to moving in specific 
 velocity to emulate rolling or moving space (like planets :)) 

## each game launch and level start the cube color , sphere colors , sphere starting position and  sphere initial direction of movement will all be generated randomly , and sphere speed of movement will be determined by the current level  .

## the snake will try to each spheres using IK solver algorithm and its stop when its collide with the picked sphere and wait for next pick  , and any sphere collided 
 with snake during the game will be collected and counted .

##bouncing spheres : 
### sphere will bounce of wall when it center + it radius out of wall meaning its going to cross the  wall then it  will bounce off it , we calculate reflection  using 
 the velocity vector and normal determined by the wall in the equation R = V - 2N(N.V)  where V is the old velocity and R is the new one , with that we emulate natural  
 bouncing off wall.
### to determine when spheres should bounce off each other we should know if they collide but we cant use the collision  detection from assignment 4 because that will make game frames drop   , but hence they same shape and can be represented in math equation so we can determine if they collide :assume c1 c2 centers of 2 spheres respectively then if destination from c1 to c2 is less than 2* radius of sphere then they are collided and from there we can calculate there reflection vectors, hence they will have same effect on each other then we can take the vector (c2-c1) as normal for sphere 1  the same for sphere 2 with (c1-c2) and using their velocity to calculate reflection just like above and create natural bouncing animation .

## adding audio , we use irrKlang free library that offer running wave and mp3 sound in program , we set listener and put listing direction (0,0,-1)
### first add 2D background sound in game when its starts .
### we also run sound when player finish level , and snake collect snake ,but this sound is 3D meaning its generated from specific position in space ,and we use the 
 center collected sphere as the  source of audio ,and when player press on explosive sphere an explosion sound will run as well .
### bouncing sound played when bouncing animation detected also 3D ,it source the sphere center or the spheres mid point , determined by the type of bouncing.

## we add second screen that will view from first person of snake and will also include UI , camera location changed to be on the head of snake and change with snake 
 head position .

## user interface : we use DearImgui to give setting choices to player and we also save score on scorefile that will save score, we read and save to it after player end 
 game and load best score from it each time player start a game .  


#################################### Resources ##########################

## base engine : https://github.com/tamirgrBGU/EngineForAnimationCourse

## audio library : https://www.ambiera.com/irrklang/

## IK materials : https://www.cs.bgu.ac.il/~vgp201/wiki.files/FABRIK.pdf

## collision detection materials : https://www.cs.bgu.ac.il/~vgp201/wiki.files/obb_sat.pdf

## dead imgui materials : https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html



   #####                 ##             #         #####
   #                    #  #           #          #    #
   #                   #    #         #           #     #
   #####              #      #       #            #     #
   #                 #        #     #             #     # 
   #                #          #   #              #    # 
   #####           ##           ##                ####


# cheats : press c and enter level in terminal  number to go to. 
     
 



     