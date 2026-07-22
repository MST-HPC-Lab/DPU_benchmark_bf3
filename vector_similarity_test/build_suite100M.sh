# ./build_suite.sh >> results/combined_log.txt 2>>&1 & disown
# nohup ./build_suite.sh > custom_output.log 2>&1 &

if grep -q "BlueField" /sys/class/dmi/id/product_name; then
    echo "DETECTED BLUEFIELD; DISCONTINUING!"

else 
    echo "Building SpaceV Indexes"
    python3 index_builder.py --file "base.100M.i8bin" --out_folder "spacev"