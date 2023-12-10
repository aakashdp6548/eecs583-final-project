# compile
clang++ eecs583-final-project/add_vecs.cpp -fopenmp -fopenmp-targets=nvptx64 -o add_vecs_cpu
clang++ eecs583-final-project/add_vecs_offload.cpp -fopenmp -fopenmp-targets=nvptx64 -o add_vecs_offload
clang++ eecs583-final-project/add_vecs_um.cpp -fopenmp -fopenmp-targets=nvptx64 -I/sw/summitdev/cuda/9.0.69/include -lcudart -I/usr/local/cuda/include -L/usr/local/bin/cuda/lib64 -o add_vecs_um

# run
./add_vecs_cpu
./add_vecs_offload
./add_vecs_um