# Generate RGB header files for all gifs in src
for i in src/*.gif; do
	new_path=$(echo $i | sed '0,/src/s//image/')
	rgb_path=${new_path%.*}.rgb
	h_path=${new_path%.*}.h
	echo "Processing $i to $h_path"
	./to_rgb.sh $i $rgb_path > /dev/null
	./to_h.sh $rgb_path $h_path > /dev/null
	rm $rgb_path
done
