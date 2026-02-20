from PIL import Image, ImageFile
import glob, os, argparse, colorama, sys

parser = argparse.ArgumentParser(description='Generates icons for various platforms')
parser.add_argument("--crop-type", help="Which side to reference when cropping.", default="shortest", type=str, choices=["shortest", "longest", "none"])
parser.add_argument("--build-dir", help="Where the build folder is located.", default="build", type=str)
parser.add_argument("--resources-dir", help="Where the resources folder is located.", default="resources", type=str)
args = parser.parse_args()

colorama.init()

def print_with_tag(color: str, tag: str, *args):
    print(
        f"{getattr(colorama.Back, color)}{colorama.Fore.BLACK} [{tag}] {colorama.Style.RESET_ALL}{getattr(colorama.Fore, color)}",
        *args,
        colorama.Style.RESET_ALL
    )

def icon_boilerplate(file: str) -> Image.Image:
    image = Image.open(file).convert("RGBA")

    if image.width != image.height:
        if (args.crop_type == "none"):
            print_with_tag("YELLOW", "WARNING", f"{file} is not a square aspect ratio ({image.width}x{image.height}), but crop-type is \"{args.crop_type}\"")
            return image

        print_with_tag("YELLOW", "WARNING", f"{file} is not a square aspect ratio ({image.width}x{image.height}), image will be cropped to {args.crop_type} side")

        if args.crop_type == "shortest": chosen_side = min(*image.size)
        if args.crop_type == "longest": chosen_side = max(*image.size)

        crop_left = (image.width - chosen_side) // 2
        crop_top = (image.height - chosen_side) // 2
        crop_right = (image.width + chosen_side) // 2
        crop_bottom = (image.height + chosen_side) // 2

        return image.crop((crop_left, crop_top, crop_right, crop_bottom))

    return image

def create_android_icon(icon: Image.Image, savename: str):
    if max(icon.width, icon.height) < 192:
        print_with_tag("YELLOW", "WARNING", f"{icon_file} is smaller than xxxhdpi (192x192)")

    for fmt in [
        { "dpi": "mdpi",    "size": 48  },
        { "dpi": "hdpi",    "size": 72  },
        { "dpi": "xhdpi",   "size": 96  },
        { "dpi": "xxhdpi",  "size": 144 },
        { "dpi": "xxxhdpi", "size": 192 }
    ]:
        save_path = f"{args.build_dir}/android/app/src/main/res/mipmap-{fmt.get('dpi')}"
        os.makedirs(save_path, exist_ok=True)

        icon.resize(
            (fmt.get('size'), round((icon.height / icon.width) * fmt.get('size'))),
            Image.Resampling.LANCZOS
        ).save(f"{save_path}/{savename}.png", "png")

        print(f"Built mipmap-{fmt.get('dpi')} for 'icon'")

files = glob.glob(f"{args.resources_dir}/App/icon.*")
for i in range(len(files)): files[i] = files[i].replace("\\", "/")

if (len(files) == 0):
    print_with_tag("RED", "ERROR", f"No file found for 'icon'")
    sys.exit()
if (len(files) > 1):
    print_with_tag("RED", "ERROR", f"More than one file found for 'icon': {', '.join(files)}")
    sys.exit()

icon_file = files[0]
print(f"Found 'icon': {icon_file}")

icon = icon_boilerplate(icon_file)

create_android_icon(icon, 'ic_launcher')

# Non-Android icons
save_path = f"{args.build_dir}/assets"
os.makedirs(save_path, exist_ok=True)

if max(icon.width, icon.height) < 256:
    print_with_tag("YELLOW", "WARNING", f"{icon_file} is smaller than hidpi (256x256)")
icon.save(f"{save_path}/icon.ico", "ico")
print(f"Built icon.ico for 'icon'")

with open(f"{save_path}/app.rc", "w") as rc:
    rc.write("IDI_ICON1 ICON DISCARDABLE \"icon.ico\"")
    print(f"Built app.rc for 'icon'")

if max(icon.width, icon.height) < 1024:
    print_with_tag("YELLOW", "WARNING", f"{icon_file} is smaller than ic10 (1024x1024)")
icon.save(f"{save_path}/icon.icns", "icns")
print(f"Built icon.icns for 'icon'")

# TODO - add UNIX icons