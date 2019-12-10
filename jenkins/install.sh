mkdir -p /OpenPhySyn/build
cd /OpenPhySyn/build
cmake ..
make -j $(nproc)
