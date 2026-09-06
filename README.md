# Software rendering in 500 lines of bare C++

The code itself is of little interest. Check the course notes:
1. [Introduction](https://haqr.eu/tinyrenderer/)
2. [Bresenham’s line drawing algorithm](https://haqr.eu/tinyrenderer/bresenham/)
3. [Triangle rasterization](https://haqr.eu/tinyrenderer/rasterization/)
4. [Primer on barycentric coordinates](https://haqr.eu/tinyrenderer/barycentric/)
5. [Hidden faces removal](https://haqr.eu/tinyrenderer/z-buffer/)
6. [A crude (but simple) approach to camera handling](https://haqr.eu/tinyrenderer/camera-naive/)
7. [Better camera handling](https://haqr.eu/tinyrenderer/camera/)
8. [Shading](https://haqr.eu/tinyrenderer/shading/)
9. [More data!](https://haqr.eu/tinyrenderer/textures/)
10. [Tangent space normal mapping](https://haqr.eu/tinyrenderer/tangent/)
11. [Shadow mapping](https://haqr.eu/tinyrenderer/shadow/)
12. [Indirect lighting](https://haqr.eu/tinyrenderer/ssao/)
13. [Bonus: toon shading](https://haqr.eu/tinyrenderer/toon/)
14. [Afterword](https://haqr.eu/tinyrenderer/afterword/)

In this series of articles, I aim to demonstrate how OpenGL, Vulkan, Metal, and DirectX work by writing a simplified clone from scratch.
Surprisingly, many people struggle with the initial hurdle of learning a 3D graphics API.
To help with this, I have prepared a short series of lectures, after which my students are able to produce quite capable renderers.

The task is as follows: using no third-party libraries (especially graphics-related ones), we will generate an image like this:

![](https://haqr.eu/tinyrenderer/home/africanhead.png)

_Warning: This is a training material that loosely follows the structure of modern 3D graphics libraries.
It is a **software renderer**.
**I do not intend to show how to write GPU applications — I want to show how they work.**
I firmly believe that understanding this is essential for writing efficient applications using 3D libraries._

## The starting point

The final code consists of about 500 lines.
My students typically require 10 to 20 hours of programming to start producing such renderers.
The input is a 3D model composed of a triangulated mesh and textures.
The output is a rendering.
There is no graphical interface, the program simply generates an image.

To minimize external dependencies, I provide my students with a single class for handling [TGA](http://en.wikipedia.org/wiki/Truevision_TGA) files —
one of the simplest formats supporting RGB, RGBA, and grayscale images.
This serves as our foundation for image manipulation.
At the beginning, the only available functionality (besides loading and saving images) is the ability to set the color of a single pixel.

There are no built-in functions for drawing line segments or triangles — we will implement all of this manually.
While I provide my own source code, written alongside my students, I do not recommend using it directly, as doing the work yourself is essential to understanding the concepts.
The complete code is available on [github](https://github.com/ssloy/tinyrenderer), and you can find the initial source code I provide to my students [here](https://github.com/ssloy/tinyrenderer/tree/706b2dfecff65daeb93de568ee2c2bd87f277860).
Behold, here is the starting point:

```cpp
#include "tgaimage.h"

constexpr TGAColor white   = {255, 255, 255, 255}; // attention, BGRA order
constexpr TGAColor green   = {  0, 255,   0, 255};
constexpr TGAColor red     = {  0,   0, 255, 255};
constexpr TGAColor blue    = {255, 128,  64, 255};
constexpr TGAColor yellow  = {  0, 200, 255, 255};

int main(int argc, char** argv) {
    constexpr int width  = 64; 
    constexpr int height = 64; 
    TGAImage framebuffer(width, height, TGAImage::RGB);

    int ax =  7, ay =  3;  
    int bx = 12, by = 37; 
    int cx = 62, cy = 53; 

    framebuffer.set(ax, ay, white);
    framebuffer.set(bx, by, white);
    framebuffer.set(cx, cy, white);

    framebuffer.write_tga_file("framebuffer.tga");
    return 0;
}
```

It produces the 64x64 image `framebuffer.tga`, here I scaled it for better readability:

![](https://haqr.eu/tinyrenderer/bresenham/bresenham0.png)


## Teaser: few examples made with the renderer

![](https://haqr.eu/tinyrenderer/home/demon.png)

![](https://haqr.eu/tinyrenderer/home/diablo-glow.png)

![](https://haqr.eu/tinyrenderer/home/boggie.png)

![](https://haqr.eu/tinyrenderer/home/diablo-ssao.png)

## Compilation

```sh
git clone https://github.com/ssloy/tinyrenderer.git &&
cd tinyrenderer &&
cmake -Bbuild &&
cmake --build build -j &&
build/tinyrenderer obj/diablo3_pose/diablo3_pose.obj obj/floor.obj
```
The rendered image is saved to `framebuffer.tga`.


## 🌐 Web Resources & Interactive Index
- [MY ARCADE CENTER](https://studyplayings.web.app/my-arcade-center.html)
- [CRAZY STUNTS 3D](https://iskillquest.pages.dev/crazy-stunts-3d.html)
- [FOREST GLADE MYSTERIES](https://iskillquest.pages.dev/forest-glade-mysteries.html)
- [SNAKE HUNTER](https://iskillplay.web.app/snake-hunter.html)
- [CATEGORY UNBLOCKER](https://quizverses-9d2f2.web.app/category-unblocker.html)
- [FAR ORION NEW WORLDS](https://iskillplay.web.app/far-orion-new-worlds.html)
- [MOJICON LOVE CONNECT](https://iskillquest.pages.dev/mojicon-love-connect.html)
- [HEAT INCREMENTAL](https://themindskillplayplay.pages.dev/heat-incremental.html)
- [CATEGORY ART](https://studyquests.github.io/category-art.html)
- [INDEX4](https://theskillquest.pages.dev/index4.html)
- [CATEGORY DRESS UP 3](https://iskillquest.pages.dev/category-dress-up-3.html)
- [CATEGORY ARENA255](https://quizverses.github.io/category-arena255.html)
- [ELLIE AND FRIENDS VENICE CARNIVAL](https://iskillplay.web.app/ellie-and-friends-venice-carnival.html)
- [CATEGORY POOL](https://iskillplay.web.app/category-pool.html)
- [CATEGORY CASUAL 4](https://iskillplay.web.app/category-casual-4.html)
- [AIRPORT SECURITY](https://theskillquest.pages.dev/airport-security.html)
- [GUN CLONE](https://iskillplay.web.app/gun-clone.html)
- [CATEGORY MATCH 3](https://themindplay.github.io/category-match-3.html)
- [COUNTRY LIFE MEADOWS](https://iskillplay.web.app/country-life-meadows.html)
- [BRAINROT HOLE](https://quizverses.github.io/brainrot-hole.html)
- [CATEGORY GUN238](https://quizverses.github.io/category-gun238.html)
- [WOOD HEXA FACTORY](https://quizverses-9d2f2.web.app/wood-hexa-factory.html)
- [SUM SHUFFLE](https://iskillquest.pages.dev/sum-shuffle.html)
- [WAR ROBOTS BATTLES](https://iskillplay.web.app/war-robots-battles.html)
- [ONLINE PORTAL](https://quizverses.github.io/)
- [CHECKERS DRAUGHTS MULTIPLAYER](https://thequizzone.pages.dev/checkers-draughts-multiplayer.html)
- [CATEGORY BUBBLE SHOOTER](https://quizverses-9d2f2.web.app/category-bubble-shooter.html)
- [SOCCER TOURNAMENT](https://thelearnquesters.pages.dev/soccer-tournament.html)
- [CATEGORY 1 PLAYER139](https://quizverses.github.io/category-1-player139.html)
- [IDLE HOTEL EMPIRE](https://learnquester.pages.dev/idle-hotel-empire.html)
- [CATEGORY MMO24](https://iskillplay.web.app/category-mmo24.html)
- [UNICORN FIND THE DIFFERENCES](https://iskillplay.web.app/unicorn-find-the-differences.html)
- [CATEGORY FPS174](https://iskillplay.web.app/category-fps174.html)
- [CUT N FILL](https://iskillplay.web.app/cut-n-fill.html)
- [BATTLE ZONE 2D](https://studyplayings.pages.dev/battle-zone-2d.html)
- [PET SALON 2](https://studyquesthub.web.app/pet-salon-2.html)
- [MATH KING MATH SKILL GAME](https://thelearnquesters.pages.dev/math-king-math-skill-game.html)
- [DOORS AWAKENING](https://thequizzone.pages.dev/doors-awakening.html)
- [HUNTER UNDERWATER SPEARFISHING](https://iskillquest.pages.dev/hunter-underwater-spearfishing.html)
- [ARROW SLIDE PUZZLE](https://iskillquest.pages.dev/arrow-slide-puzzle.html)
- [RAGDOLL ARENA 2 PLAYER](https://themindzone.pages.dev/ragdoll-arena-2-player.html)
- [SAVE STRANDING FISH](https://thequizzone.pages.dev/save-stranding-fish.html)
- [COOL CARS RACING AT ALTITUDE](https://studyplayings.pages.dev/cool-cars-racing-at-altitude.html)
- [COUNTER CRAFT SNIPER](https://quizverses.github.io/counter-craft-sniper.html)
- [PANDA ADVENTURE](https://iskillplay.web.app/panda-adventure.html)
- [CATEGORY BATTLE 2](https://iskillplay.web.app/category-battle-2.html)
- [CATEGORY CASUAL 7](https://iskillplay.web.app/category-casual-7.html)
- [POCKET CAR MASTER](https://thelearnquesters.pages.dev/pocket-car-master.html)
- [DELTA FORCE AIRBORNE](https://theskillquest.pages.dev/delta-force-airborne.html)
- [LIMOUSINE CAR GAME SIMULATOR](https://thelearnquester.web.app/limousine-car-game-simulator.html)
- [WILD WEST MATCH](https://iskillplay.web.app/wild-west-match.html)
- [EXCAVATOR SIMULATOR 3D](https://learnquesters.pages.dev/excavator-simulator-3d.html)
- [HIDDEN OBJECTS](https://studyplayings.web.app/hidden-objects.html)
- [PUSH THE COLORS](https://quizverses.github.io/push-the-colors.html)
- [HARBOR OPERATOR](https://themindzone.pages.dev/harbor-operator.html)
- [CITY BUILDER](https://iskillplay.web.app/city-builder.html)
- [SUMMER RIDER 3D](https://theskillquest.pages.dev/summer-rider-3d.html)
- [CATEGORY MISSION207](https://thelearnquesters.pages.dev/category-mission207.html)
- [FARMER RUSH IDLE FARM GAME](https://theskillquest.pages.dev/farmer-rush-idle-farm-game.html)
- [FIRE TRUCK DRIVING SIMULATOR](https://iskillplay.web.app/fire-truck-driving-simulator.html)
- [HERO RABBIT IDLE SURVIVOR RPG](https://thelearnquesters.pages.dev/hero-rabbit-idle-survivor-rpg.html)
- [THE FINAL RIDDLE](https://iskillplay.web.app/the-final-riddle.html)
- [NUTS BOLTS SORT COLOR PUZZLE](https://thequizzone.pages.dev/nuts-bolts-sort-color-puzzle.html)
- [STELLAR MINES SPACE MINER](https://studyquests.github.io/stellar-mines-space-miner.html)
- [CATEGORY MINECRAFT 2](https://studyplayings.pages.dev/category-minecraft-2.html)
- [WINTER WOLF](https://learnquester.pages.dev/winter-wolf.html)
- [SIBERIAN ASSAULT](https://quizverses.github.io/siberian-assault.html)
- [FRUITSLAND ESCAPE FROM THE AMUSEMENT PARK](https://thequizzone.pages.dev/fruitsland-escape-from-the-amusement-park.html)
- [CATEGORY FARMING87](https://iskillplay.web.app/category-farming87.html)
- [BANG BANG MAHJONG](https://iskillplay.web.app/bang-bang-mahjong.html)
- [SAVE MY PET](https://learnquester.pages.dev/save-my-pet.html)
- [CATEGORY SURVIVAL366](https://studyplayings.pages.dev/category-survival366.html)
- [DOCTOR CHICKEN](https://quizverses.github.io/doctor-chicken.html)
- [FROGGY HOP](https://studyquests.github.io/froggy-hop.html)
- [BOOST YOUR BRAIN](https://themindskillplayplay.pages.dev/boost-your-brain.html)
- [THATS NOT MY NEIGHBOR](https://quizverses.github.io/thats-not-my-neighbor.html)
- [INDEX32](https://themindskillplayplay.pages.dev/index32.html)
- [FOOTBALL PENALTY 2026](https://studyquests.github.io/football-penalty-2026.html)
- [FLICK BASEBALL SUPER HOMERUN](https://learnquester.pages.dev/flick-baseball-super-homerun.html)
- [ZINDEX](https://learnquester.pages.dev/zindex.html)
- [CATEGORY IDLE445](https://themindplay.github.io/category-idle445.html)
- [DIAMONDZ](https://studyquests.github.io/diamondz.html)
- [BLASTIFY II](https://learnquesters.pages.dev/blastify-ii.html)
- [LABUBU MERGE](https://studyquests.github.io/labubu-merge.html)
- [OBBY MASSIVE ATTACK](https://learnquesters.pages.dev/obby-massive-attack.html)
- [CATEGORY CASUAL 3](https://quizverses-9d2f2.web.app/category-casual-3.html)
- [BALLOON POP FRENZY](https://iskillplay.web.app/balloon-pop-frenzy.html)
- [KNOCK AND RUN 100 DOORS ESCAPE](https://learnquesters.pages.dev/knock-and-run-100-doors-escape.html)
- [DOP PUZZLE ERASE MASTER](https://studyquests.github.io/dop-puzzle-erase-master.html)
- [DAILY HOROSCOPE HD](https://studyplayings.pages.dev/daily-horoscope-hd.html)
- [JENNYS MATH PUZZLE](https://iskillplay.web.app/jennys-math-puzzle.html)
- [CATEGORY WAR137](https://iskillplay.web.app/category-war137.html)
- [CATEGORY MERGE221](https://iskillquest.pages.dev/category-merge221.html)
- [FRAY FIGHT](https://studyquests.github.io/fray-fight.html)
- [INDEX2](https://quizverses.github.io/index2.html)
- [MAHJONG TRIPLE 3D TILE MATCH](https://studyplaying.github.io/mahjong-triple-3d-tile-match.html)
- [TANK BATTLEIO](https://quizverses.github.io/tank-battleio.html)
- [ALIEN WRECKAGE 1](https://studyquests.github.io/alien-wreckage-1.html)
- [SNAKE KING](https://quizverses-9d2f2.web.app/snake-king.html)
- [SANTA VS SKRITCH](https://thequizzone.pages.dev/santa-vs-skritch.html)
- [LULUS FASHION WORLD](https://thelearnquesters.pages.dev/lulus-fashion-world.html)
- [CLICKER HEROES](https://iskillplay.web.app/clicker-heroes.html)
- [BATTLEDUDES IO](https://studyplayings.web.app/battledudes-io.html)
- [SHELL STRIKERS](https://thelearnquesters.pages.dev/shell-strikers.html)
- [ONE LINE DRAWING](https://iskillplay.web.app/one-line-drawing.html)
- [WORD SEARCH UNIVERSE](https://learnquester.pages.dev/word-search-universe.html)
- [BUBBLE SHOOTER VINTAGE](https://quizverses-9d2f2.web.app/bubble-shooter-vintage.html)
- [TAPE SORT 3D](https://thequizzone.pages.dev/tape-sort-3d.html)
- [DARK ACADEMIA WEDDING](https://iskillquest.pages.dev/dark-academia-wedding.html)
- [RED LIGHT GREEN LIGHT](https://thequizzone.pages.dev/red-light-green-light.html)
- [WOOP CRAWL UP](https://iskillplay.web.app/woop-crawl-up.html)
- [THEO MORINIS MAGICAL RESORT](https://iskillplay.web.app/theo-morinis-magical-resort.html)
- [CONQUERIO](https://studyquests.github.io/conquerio.html)
- [EMOJI SORT FUN PUZZLE GAME](https://quizverses.github.io/emoji-sort-fun-puzzle-game.html)
- [CLEAN HOUSE CLEARING TRASH AND DIRT](https://quizverses.github.io/clean-house-clearing-trash-and-dirt.html)
- [INDEX11](https://quizverses.github.io/index11.html)
- [CANNON SHOOTER](https://theskillquest.pages.dev/cannon-shooter.html)
- [BUBBLE SHOOTER BUTTERFLY](https://theskillquest.pages.dev/bubble-shooter-butterfly.html)
- [KAWAII FRIENDS TILES MATCHER](https://themindplay.github.io/kawaii-friends-tiles-matcher.html)
- [NUMBER RUSH](https://studyplayings.pages.dev/number-rush.html)
- [BUTTERFLY SORT PUZZLE](https://studyplaying.github.io/butterfly-sort-puzzle.html)
- [DOWNHILL CAR RIDE CRASH TEST](https://studyplayings.web.app/downhill-car-ride-crash-test.html)
- [DEAD BRAIN](https://studyplayings.web.app/dead-brain.html)
- [PIM PATH](https://iskillplay.web.app/pim-path.html)
- [LIFE CLICKER](https://thequizzone.pages.dev/life-clicker.html)
- [CATEGORY ESCAPE 2](https://iskillplay.web.app/category-escape-2.html)
- [CATEGORY BATTLE ROYALE25](https://theskillquest.pages.dev/category-battle-royale25.html)
- [BEAM DRIVE CAR CRASH TEST SIMULATOR](https://studyplayings.pages.dev/beam-drive-car-crash-test-simulator.html)
- [MOJICON LOVE CONNECT](https://thequizzone.pages.dev/mojicon-love-connect.html)
- [SOCCER DASH](https://studyplayings.web.app/soccer-dash.html)
