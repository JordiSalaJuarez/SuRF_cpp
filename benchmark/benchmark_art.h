#include <benchmark/benchmark.h>
#include <art.h>
#include <cstring>
#include <gsl/gsl_util>
#include "utils.h"


auto n_bytes(art_node *n) -> std::size_t{
    union {
        art_node4 *p1;
        art_node16 *p2;
        art_node48 *p3;
        art_node256 *p4;
    } p;
    std::size_t sum_bytes = 0;
    switch (n->type) {
        case NODE4:
            p.p1 = reinterpret_cast<art_node4*>(n);
            for (std::size_t i=0 ; i < n->num_children; i++) {
                    sum_bytes += n_bytes(p.p1->children[i]);
            }
            return sizeof(*p.p1) + sum_bytes;
        case NODE16:
            p.p2 = reinterpret_cast<art_node16*>(n);
            for (std::size_t i=0 ; i < n->num_children; i++) {
                    sum_bytes += n_bytes(p.p2->children[i]);
            }
            return sizeof(*p.p2) + sum_bytes;

        case NODE48:
            p.p3 = reinterpret_cast<art_node48*>(n);
            for (std::size_t i=0 ; i < n->num_children; i++) {
                    sum_bytes += n_bytes(p.p3->children[i]);
            }
            return sizeof(*p.p3) + sum_bytes;

        case NODE256:
            p.p4 = reinterpret_cast<art_node256*>(n);
            for (std::size_t i=0 ; i < n->num_children; i++) {
                    sum_bytes += n_bytes(p.p4->children[i]);
            }
            return sizeof(*p.p4) + sum_bytes;
        default:
            return sizeof(*n);
    }
}

auto n_bytes(art_tree *tree) -> std::size_t{
    return sizeof(*tree) + n_bytes(tree->root);
}


static void BM_ConstructionART(benchmark::State & state){
    auto keys = get_input_data(state.range());
    for (auto _ : state){
        art_tree t;
        int res = art_tree_init(&t);
        assert(res == 0);
        auto line = 1;
        for (const auto& key: keys){
            void * value;
            memcpy(&value, &line, sizeof(void *));
            art_insert(&t, (const unsigned char *) key.c_str(),std::size(key), value);
            ++line;
        }
        res = art_tree_destroy(&t);
        assert(res == 0);
    }
    art_tree t;
    int res = art_tree_init(&t);
    assert(res == 0);
    auto line = 1;
    for (const auto& key: keys){
        void * value;
        memcpy(&value, &line, sizeof(void *));
        art_insert(&t, (const unsigned char *) key.c_str(),std::size(key), value);
        ++line;
    }
    state.counters["Size(MB)"] = gsl::narrow_cast<double>(n_bytes(&t) / 1024 / 1024);
    res = art_tree_destroy(&t);
}


static void BM_PointQueryART(benchmark::State & state){
    auto keys = get_input_data(state.range());
    art_tree t;
    int res = art_tree_init(&t);
    assert(res == 0);
    auto line = 1;
    for (const auto& key: keys){
        void * value;
        memcpy(&value, &line, sizeof(void *));
        art_insert(&t, (const unsigned char *) key.c_str(),std::size(key), value);
        ++line;
    }
    for (auto _ : state){
        auto key = keys[rand()%size(keys)];
        uintptr_t val = (uintptr_t)art_search(&t, (unsigned char*)key.c_str(), std::size(key));
        benchmark::DoNotOptimize(val);
    }
    state.counters["Size(MB)"] = gsl::narrow_cast<double>(n_bytes(&t) / 1024 / 1024);
    res = art_tree_destroy(&t);
    assert(res == 0);
}