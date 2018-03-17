# AMVP-Gen

ArduMan Video Generator

## Usage

Using `paletteuse` to dither:
```
ffmpeg -i "*input-file*" -i "*input-palette*" -lavfi "scale=128:64:flags=lanczos [x];[x][1:v] paletteuse" -pix_fmt bgr24 -f rawvideo - | amvpgen -x 128 -y 64 -f 4 out.bin
```
Where `*input-palette*` is a pallete for dither.

Using no dither:
```
ffmpeg -i "*input-file*" -lavfi "scale=128:64:flags=lanczos" -pix_fmt bgr24 -f rawvideo - | amvpgen -x 128 -y 64 -f 4 out.bin
```
