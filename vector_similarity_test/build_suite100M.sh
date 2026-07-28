# ./build_suite.sh >> results/combined_log.txt 2>>&1 & disown
# nohup ./build_suite.sh > custom_output.log 2>&1 &

if grep -q "BlueField" /sys/class/dmi/id/product_name; then
    echo "DETECTED BLUEFIELD; DISCONTINUING!"

else 
    #echo "Building SpaceV Indexes"
    #python3 index_builder.py --file "base.100M.i8bin" --out_folder "spacev"

    echo "Building SpaceV Indexes (flat)"
    python3 index_builder.py --only "flat" --file "base.100M.i8bin" --out_folder "spacev" --test_stride 1000 
    echo "Building SpaceV Indexes (lsh)"
    python3 index_builder.py --only "lsh" --file "base.100M.i8bin" --out_folder "spacev"  --test_stride 1000
    echo "Building SpaceV Indexes(pq)"
    python3 index_builder.py --only "pq" --file "base.100M.i8bin" --out_folder "spacev" --test_stride 1000
    echo "Building SpaceV Indexes(ivfpq)"
    python3 index_builder.py --only "ivfpq" --file "base.100M.i8bin" --out_folder "spacev"  --test_stride 1000
    echo "Building SpaceV Indexes(hsnw)"
    python3 index_builder.py --only "hsnw" --file "base.100M.i8bin" --out_folder "spacev"  --test_stride 1000
    echo "Building SpaceV Indexes(hsnw_pq)"
    python3 index_builder.py --only "hsnw_pq" --file "base.100M.i8bin" --out_folder "spacev"  --test_stride 1000
    echo "Building SpaceV Indexes(hsnw_sq)"
    python3 index_builder.py --only "hsnw_sq" --file "base.100M.i8bin" --out_folder "spacev"  --test_stride 1000
fi