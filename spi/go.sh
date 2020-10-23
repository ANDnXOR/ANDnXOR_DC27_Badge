#Start fresh
rm -rf image
mkdir image

#Copy static files
cp -R src/TUNEZ image
cp -R src/ETC image

#Generate RGB files
./gen_rgb.sh
