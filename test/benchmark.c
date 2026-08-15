#include <easyc.h>
#include <time.h>

#ifdef BENCHMARK

static float int_score(int x) { return (float)x; }

static double get_time() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000.0 + ts.tv_nsec / 1e6;
}

static double arraylist_carousel(size_t arrs, size_t depth) {
    double time = get_time();
    ARRLIST_int* arrlists = EZ_ALLOC(arrs, sizeof(ARRLIST_int));
    for (size_t i = 0; i < depth; i++) {
        ARRLIST_int_add(&(arrlists[rand()%arrs]), 0xFFFFF);
        ARRLIST_int_clear(&(arrlists[rand()%arrs]));
    }
    for (size_t i = 0; i < arrs; i++) {
        ARRLIST_int_clear(&(arrlists[i]));
    }
    EZ_FREE(arrlists);
    return get_time() - time;
}

static void fill_sorted(ARRLIST_int* list, size_t n) {
    ARRLIST_int_clear(list);
    for (size_t i = 0; i < n; i++) ARRLIST_int_add(list, (int)i);
}

static void fill_reversed(ARRLIST_int* list, size_t n) {
    ARRLIST_int_clear(list);
    for (size_t i = n; i > 0; i--) ARRLIST_int_add(list, (int)i);
}

static void fill_random(ARRLIST_int* list, size_t n) {
    ARRLIST_int_clear(list);
    for (size_t i = 0; i < n; i++) ARRLIST_int_add(list, rand());
}

static void fill_nearly_sorted(ARRLIST_int* list, size_t n, int swap_pct) {
    ARRLIST_int_clear(list);
    for (size_t i = 0; i < n; i++) ARRLIST_int_add(list, (int)i);
    size_t swaps = (n * swap_pct) / 100;
    for (size_t i = 0; i < swaps; i++) {
        size_t a = rand() % n;
        size_t b = rand() % n;
        int tmp = list->data[a];
        list->data[a] = list->data[b];
        list->data[b] = tmp;
    }
}

static void fill_constant(ARRLIST_int* list, size_t n) {
    ARRLIST_int_clear(list);
    for (size_t i = 0; i < n; i++) ARRLIST_int_add(list, 42);
}

static void fill_single_outlier_front(ARRLIST_int* list, size_t n) {
    ARRLIST_int_clear(list);
    ARRLIST_int_add(list, (int)n * 9999);
    for (size_t i = 1; i < n; i++) ARRLIST_int_add(list, (int)i);
}

static void fill_sawtooth(ARRLIST_int* list, size_t n, size_t period) {
    ARRLIST_int_clear(list);
    for (size_t i = 0; i < n; i++) ARRLIST_int_add(list, (int)(i % period));
}

static double bench_easysort(void (*fill)(ARRLIST_int*, size_t), size_t n, int runs) {
    ARRLIST_int list = { 0 };
    double total = 0.0;
    for (int r = 0; r < runs; r++) {
        fill(&list, n);
        double t = get_time();
        EasySort_int(&list, int_score);
        total += get_time() - t;
    }
    ARRLIST_int_clear(&list);
    return total / runs;
}

static double bench_easysort_nearly(size_t n, int swap_pct, int runs) {
    ARRLIST_int list = { 0 };
    double total = 0.0;
    for (int r = 0; r < runs; r++) {
        fill_nearly_sorted(&list, n, swap_pct);
        double t = get_time();
        EasySort_int(&list, int_score);
        total += get_time() - t;
    }
    ARRLIST_int_clear(&list);
    return total / runs;
}

static double bench_easysort_sawtooth(size_t n, size_t period, int runs) {
    ARRLIST_int list = { 0 };
    double total = 0.0;
    for (int r = 0; r < runs; r++) {
        fill_sawtooth(&list, n, period);
        double t = get_time();
        EasySort_int(&list, int_score);
        total += get_time() - t;
    }
    ARRLIST_int_clear(&list);
    return total / runs;
}

int main(int argc, const char** argv) {
    EZ_INFO("Starting benchmark suite...\n");
    srand(time(NULL));
    EZ_INFO("Benchmarking default memory tech using \"arraylist_carousel()\"\n");
    EZ_INFO("10x10         | %.6f ms", arraylist_carousel(10, 10));
    EZ_INFO("100x100       | %.6f ms", arraylist_carousel(100, 100));
    EZ_INFO("1000x1000     | %.6f ms", arraylist_carousel(1000, 1000));
    EZ_INFO("10000x10000   | %.6f ms", arraylist_carousel(10000, 10000));
    EZ_INFO("100000x100000 | %.6f ms", arraylist_carousel(100000, 100000));
    EZ_INFO("10x100000     | %.6f ms\n", arraylist_carousel(10, 100000));

    int SORT_RUNS = 5;
    EZ_INFO("Benchmarking EasySort — averaged over %d runs\n", SORT_RUNS);

    EZ_INFO("--- Already sorted (best case) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort(fill_sorted, 100,    SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort(fill_sorted, 1000,   SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort(fill_sorted, 10000,  SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort(fill_sorted, 100000, SORT_RUNS));

    EZ_INFO("--- Fully reversed (worst case: all displaced) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort(fill_reversed, 100,    SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort(fill_reversed, 1000,   SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort(fill_reversed, 10000,  SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort(fill_reversed, 100000, SORT_RUNS));

    EZ_INFO("--- Random ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort(fill_random, 100,    SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort(fill_random, 1000,   SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort(fill_random, 10000,  SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort(fill_random, 100000, SORT_RUNS));

    EZ_INFO("--- Nearly sorted (1%% displaced) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort_nearly(100,    1, SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort_nearly(1000,   1, SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort_nearly(10000,  1, SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort_nearly(100000, 1, SORT_RUNS));

    EZ_INFO("--- Nearly sorted (10%% displaced) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort_nearly(100,    10, SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort_nearly(1000,   10, SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort_nearly(10000,  10, SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort_nearly(100000, 10, SORT_RUNS));

    EZ_INFO("--- Nearly sorted (50%% displaced) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort_nearly(100,    50, SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort_nearly(1000,   50, SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort_nearly(10000,  50, SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort_nearly(100000, 50, SORT_RUNS));

    EZ_INFO("--- All equal (no removals, binary search still runs on reinsertion) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort(fill_constant, 100,    SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort(fill_constant, 1000,   SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort(fill_constant, 10000,  SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort(fill_constant, 100000, SORT_RUNS));

    EZ_INFO("--- Single outlier at front (n-1 elements displaced) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort(fill_single_outlier_front, 100,    SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort(fill_single_outlier_front, 1000,   SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort(fill_single_outlier_front, 10000,  SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort(fill_single_outlier_front, 100000, SORT_RUNS));

    EZ_INFO("--- Sawtooth period=2 (alternating, maximum churn) ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort_sawtooth(100,    2, SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort_sawtooth(1000,   2, SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort_sawtooth(10000,  2, SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort_sawtooth(100000, 2, SORT_RUNS));

    EZ_INFO("--- Sawtooth period=10 ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort_sawtooth(100,    10, SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort_sawtooth(1000,   10, SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort_sawtooth(10000,  10, SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort_sawtooth(100000, 10, SORT_RUNS));

    EZ_INFO("--- Sawtooth period=100 ---");
    EZ_INFO("n=100        | %.6f ms", bench_easysort_sawtooth(100,    100, SORT_RUNS));
    EZ_INFO("n=1000       | %.6f ms", bench_easysort_sawtooth(1000,   100, SORT_RUNS));
    EZ_INFO("n=10000      | %.6f ms", bench_easysort_sawtooth(10000,  100, SORT_RUNS));
    EZ_INFO("n=100000     | %.6f ms\n", bench_easysort_sawtooth(100000, 100, SORT_RUNS));

    return 0;
}

#endif
