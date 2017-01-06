# mph-model-viewer

**[DOWNLOAD > https://github.com/McKay42/mph-model-viewer/releases](https://github.com/McKay42/mph-model-viewer/releases)**

View/export the models and textures found in [Metroid Prime Hunters](https://en.wikipedia.org/wiki/Metroid_Prime_Hunters).

You can also walk around the models (standard first-person controls), [Bullet](http://bulletphysics.org/) is used as a physics engine for player movement.

* Exports a [Collada](https://en.wikipedia.org/wiki/COLLADA) file for models, and PNG files for textures
* **Vertex colors** must be supported when importing the collada file elsewhere (for the lightmaps to work)
* If your modeling software doesn't support vertex colors in collada files, use [Blender](https://www.blender.org/) to import and then export in a different format which does support them
* If you want the exported models to look like they do in-game, create a multiplication shader/node for every material/texture which multiplies the interpolated vertex color (lightmap) with the albedo color (texture)
* Character models don't really work, since all bones are centered at ```(0,0,0)```, and their textures are broken
* Animations are not supported

Runs on McEngine: [https://github.com/McKay42/McEngine](https://github.com/McKay42/McEngine)

## Usage
* Copy your ```*.bin``` model and texture files into ```/<ModelViewer>/mph/models/``` and ```/<ModelViewer>/mph/textures/``` respectively
* Click on [Open] and navigate to ```/<ModelViewer>/mph/models/```, then select the model you want to view
* Exported files will go into ```/<ModelViewer>/exports/```
* Keep the exported collada and png files together in the same directory (for importing)

## Controls
* [W][A][S][D] = Move
* [Space] = Jump
* [Shift] = Sprint
* [Ctrl] = Crouch
* ... all other hotkeys are visible on-screen

## Screenshots
![screenshot1_modelviewer](/screenshots/mph3.PNG?raw=true)
![screenshot2_ds](/screenshots/mph3_ds.png?raw=true)
-
![screenshot3_modelviewer](/screenshots/mph2.PNG?raw=true)
![screenshot4_ds](/screenshots/mph2_ds.png?raw=true)
-
![screenshot5_modelviewer](/screenshots/mph1.PNG?raw=true)

## Blender Help
![screenshot6_blender1](/screenshots/blender1.png?raw=true)
![screenshot7_blender2](/screenshots/blender2.jpg?raw=true)
