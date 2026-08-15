#include <easyc.h>

static int passed_tests = 0;
static int total_tests = 0;

#define EZTEST(condition, name) { \
	total_tests++; \
	if (condition) { \
		printf("%d. %s - %s[PASSED]%s\n", (int)total_tests, name, EZ_GREEN, EZ_RESET); \
		passed_tests++; \
	} else { \
		printf("%d. %s - %s[FAILED]%s\n", (int)total_tests, name, EZ_RED, EZ_RESET); \
	} \
}

DECLARE_ARRLIST_NAMED(intPtr, int*);
IMPL_ARRLIST_NAMED(intPtr, int*);
DECLARE_HASHMAP(int, char, int2char);
IMPL_HASHMAP(int, char, int2char, ez_hash_int);

typedef struct {
	int* sum;
	int a;
	int b;
	EZ_MUTEX mutex;
} Params;

typedef struct {
	int32_t a;
	char b;
	float c;
	uint32_t d;
} Packet;

static EZ_THREAD_RETURN_TYPE thread_function(EZ_THREAD_PARAMETER_TYPE params) {
	Params* p = (Params*)params;
	*(p->sum) = p->a + p->b;
	return 0;
}

static EZ_THREAD_RETURN_TYPE unsafe_function(EZ_THREAD_PARAMETER_TYPE params) {
	Params* p = (Params*)params;
	EZ_LOCK_MUTEX(p->mutex);
	*(p->sum) = *(p->sum) + 1;
	EZ_RELEASE_MUTEX(p->mutex);
	return 0;
}

static float int_score(int x) { return (float)x; }
static float int_score_neg(int x) { return -(float)x; }

int main() {
	// easybool tests
	EZTEST(TRUE == 1, "Truth bool");
	EZTEST(FALSE == 0, "False bool");
	EZTEST(sizeof(BOOL) == sizeof(int), "Bool size");
	
	// easymemory tests
	void* obj = EZ_ALLOC(1, 100);
	EZTEST(obj != NULL, "Successful allocation");
	EZTEST(EZ_ALLOCATED() == 100 + sizeof(size_t), "Allocated size");
    obj = EZ_REALLOC(obj, 1, 10);
    EZTEST(EZ_ALLOCATED() == 10 + sizeof(size_t), "Reallocated smaller size");
    obj = EZ_REALLOC(obj, 1, 1000);
    EZTEST(EZ_ALLOCATED() == 1000 + sizeof(size_t), "Reallocated larger size");
    void* robj = EZ_ALLOC(1, 1000);
    memset(robj, 0, 1000);
    EZTEST(memcmp(obj, robj, 1000) == 0, "Preset reallocation to zero");
    memset(obj, 1, 1000);
    memset(robj, 1, 500);
    obj = EZ_REALLOC(obj, 1, 500);
    EZTEST(memcmp(obj, robj, 500) == 0, "Reallocation unchanged data");
    EZ_FREE(robj);
    obj = EZ_REALLOC(obj, 5, 10);
    EZTEST(EZ_ALLOCATED() == 50 + sizeof(size_t), "Multiple reallocation");
	EZ_FREE(obj);
	EZTEST(EZ_ALLOCATED() == 0, "Free memory");
	obj = EZ_ALLOC(2, 100);
	EZTEST(EZ_ALLOCATED() == 200 + sizeof(size_t), "Multiplicative allocation");
	void* obj2 = EZ_ALLOC(2, 100);
	memset(obj2, 0, 200);
	EZTEST(memcmp(obj, obj2, 200) == 0, "Preset to zero");
	EZ_FREE(obj);
	EZ_FREE(obj2);

	// easyobjects tests
	size_t before_eo_tests = EZ_ALLOCATED();
	ARRLIST_int list = { 0 };
	EZTEST(list.size == 0, "Empty list");
	EZTEST(list.maxsize == 0, "Empty list capacity");
	ARRLIST_int_add(&list, 2);
	ARRLIST_int_add(&list, 6);
	ARRLIST_int_add(&list, 10);
	ARRLIST_int_insert(&list, 4, 1);
	ARRLIST_int_insert(&list, 8, 3);
	EZTEST(list.size == 5, "Filled list");
	EZTEST(list.maxsize == 8, "List capacity");
	EZTEST(ARRLIST_int_has(&list, 10), "List contains");
	EZTEST(!ARRLIST_int_has(&list, 5), "List does not contains");
	ARRLIST_int_remove(&list, 4);
	EZTEST(!ARRLIST_int_has(&list, 10), "List removal");
	EZTEST(list.size == 4, "List removed size");
	EZTEST(list.maxsize == 8, "List maintained capacity");
	EZTEST(ARRLIST_int_get(&list, 2) == 6, "List get");
	ARRLIST_int_clear(&list);
	EZTEST(list.size == 0, "List clear size");
	EZTEST(list.maxsize == 0, "List clear capacity");
    ARRLIST_int_zero(&list, 123456);
    EZTEST(list.size == 123456, "List zero size");
    EZTEST(list.maxsize == 123456, "List zero maxsize");
    int success = 1;
    for (int i = 0; i < 123456; i++) {
        if (list.data[i] != 0) success = 0;
    }
    EZTEST(success == 1, "List zero zero'd");
    ARRLIST_int_clear(&list);
	ARRLIST_intPtr nlist = { 0 };
	EZTEST(nlist.size == 0, "Empty named list");
	EZTEST(nlist.maxsize == 0, "Empty named list capacity");
	int i = 1;
	int j = 3;
	int k = 5;
	int z = 1000;
	ARRLIST_intPtr_add(&nlist, &i);
	ARRLIST_intPtr_add(&nlist, &j);
	ARRLIST_intPtr_add(&nlist, &k);
	EZTEST(nlist.size == 3, "Filled named list");
	EZTEST(nlist.maxsize == 4, "Named list capacity");
	EZTEST(ARRLIST_intPtr_has(&nlist, &j), "Named list contains");
	EZTEST(!ARRLIST_intPtr_has(&nlist, &z), "Named list does not contains");
	ARRLIST_intPtr_remove(&nlist, 1);
	EZTEST(!ARRLIST_intPtr_has(&nlist, &j), "Named list removal");
	EZTEST(nlist.size == 2, "Named list removed size");
	EZTEST(nlist.maxsize == 4, "Named list maintained capacity");
	EZTEST(ARRLIST_intPtr_get(&nlist, 1) == &k, "Named list get");
	ARRLIST_intPtr_clear(&nlist);
	EZTEST(nlist.size == 0, "Named list clear size");
	EZTEST(nlist.maxsize == 0, "Named list clear capacity");
    ARRLIST_intPtr_zero(&nlist, 123456);
    EZTEST(nlist.size == 123456, "Named list zero size");
    EZTEST(nlist.maxsize == 123456, "Named list zero maxsize");
    success = 1;
    for (int i = 0; i < 123456; i++) {
        if (nlist.data[i] != NULL) success = 0;
    }
    EZTEST(success == 1, "Named list zero zero'd");
    ARRLIST_intPtr_clear(&nlist);
    HASHMAP_int2char hm = { 0 };
    EZTEST(hm.size == 0, "Empty hashmap");
    EZTEST(hm.capacity == 0, "Empty hashmap capacity");
    EZTEST(!HASHMAP_int2char_has(&hm, 99), "Empty hashmap has");
    HASHMAP_int2char_set(&hm, 99, 'a');
    HASHMAP_int2char_set(&hm, 69, 'b');
    EZTEST(!HASHMAP_int2char_has(&hm, 67), "Non-empty hashmap has on non-existent key");
    HASHMAP_int2char_set(&hm, 67, 'c');
    HASHMAP_int2char_set(&hm, 21, 'd');
    EZTEST(hm.size == 4, "Filled hashmap size");
    EZTEST(hm.capacity == 8, "Filled hashmap capacity");
    success = 
        HASHMAP_int2char_has(&hm, 99) &&
        HASHMAP_int2char_has(&hm, 69) &&
        HASHMAP_int2char_has(&hm, 67) &&
        HASHMAP_int2char_has(&hm, 21);
    EZTEST(success, "Filled hashmap has");
    success =
        HASHMAP_int2char_has(&hm, 1) ||
        HASHMAP_int2char_has(&hm, 98) ||
        HASHMAP_int2char_has(&hm, 100) ||
        HASHMAP_int2char_has(&hm, 2);
    EZTEST(!success, "Filled hashmap (does not) has");
    EZTEST(HASHMAP_int2char_get(&hm, 99) == 'a', "Filled hashmap get 1");
    EZTEST(HASHMAP_int2char_get(&hm, 69) == 'b', "Filled hashmap get 2");
    EZTEST(HASHMAP_int2char_get(&hm, 67) == 'c', "Filled hashmap get 3");
    EZTEST(HASHMAP_int2char_get(&hm, 21) == 'd', "Filled hashmap get 4");
    HASHMAP_int2char_set(&hm, 99, 'z');
    EZTEST(HASHMAP_int2char_get(&hm, 99) == 'z', "Hashmap replace key-value pair");
    HASHMAP_int2char_remove(&hm, 99);
    EZTEST(!HASHMAP_int2char_has(&hm, 99), "Filled hashmap remove 1");
    HASHMAP_int2char_remove(&hm, 69);
    EZTEST(!HASHMAP_int2char_has(&hm, 69), "Filled hashmap remove 2");
    HASHMAP_int2char_remove(&hm, 67);
    EZTEST(!HASHMAP_int2char_has(&hm, 67), "Filled hashmap remove 3");
    HASHMAP_int2char_remove(&hm, 21);
    EZTEST(!HASHMAP_int2char_has(&hm, 21), "Filled hashmap remove 4");
    EZTEST(hm.size == 0, "Cleared out hashmap");
    HASHMAP_int2char_clear(&hm);
    EZTEST(hm.capacity == 0, "Cleared out hashmap capacity");
    PQUEUE_int pqi = { 0 };
    EZTEST(pqi.size == 0, "Empty priority queue");
    EZTEST(pqi.capacity == 0, "Empty priority queue capacity");
    PQUEUE_int_insert(&pqi, 1, 9.0f);
    PQUEUE_int_insert(&pqi, 2, 8.0f);
    PQUEUE_int_insert(&pqi, 3, 7.0f);
    PQUEUE_int_insert(&pqi, 4, 6.0f);
    PQUEUE_int_insert(&pqi, 5, 5.0f);
    PQUEUE_int_insert(&pqi, 6, 4.0f);
    PQUEUE_int_insert(&pqi, 7, 3.0f);
    PQUEUE_int_insert(&pqi, 8, 2.0f);
    PQUEUE_int_insert(&pqi, 9, 1.0f);
    PQUEUE_int_insert(&pqi, 10, 0.0f);
    EZTEST(pqi.size == 10, "Filled priority queue size");
    EZTEST(pqi.capacity == 16, "Filled priority queue capacity");
    size_t t = pqi.size;
    int curr = 10;
    int topv = 1;
    for (size_t i = 0; i < t; i++) {
        topv |= (PQUEUE_int_top(&pqi) == curr);
        int new = PQUEUE_int_pop(&pqi);
        if (new != curr) {
            curr = -1;
            break;
        }
        curr--;
    }
    EZTEST(topv, "Priority queue top");
    EZTEST(curr == 0, "Priority queue insert and pop");
    EZTEST(pqi.size == 0, "Popped out priority queue size");
    PQUEUE_int_clear(&pqi);
    PQPAIR_int buildarr[] = {
        {92, 8.0f}, {98, 2.0f}, {90, 10.0f}, {94, 6.0f}, {97, 3.0f},
        {96, 4.0f}, {93, 7.0f}, {99, 1.0f}, {91, 9.0f}, {95, 5.0f}
    };
    int verifyarr[] = { 93, 94, 91, 99, 98, 97, 96, 95, 92, 90 };
    PQUEUE_int_build(&pqi, buildarr, 10);
    EZTEST(pqi.size == 10, "Built priority queue size");
    EZTEST(pqi.capacity == 10, "Built priority queue capacity");
    success = 1;
    for (size_t i = 0; i < pqi.size; i++)
        success &= PQUEUE_int_pop(&pqi) == 99 - (int)i;
    EZTEST(success, "Built priority queue order");
    PQUEUE_int_clear(&pqi);
    EZTEST(pqi.size == 0, "Cleared priority queue size");
    EZTEST(pqi.capacity == 0, "Cleared priority queue capacity");
    PQUEUE_int_build(&pqi, buildarr, 10);
    PQUEUE_int_update(&pqi, 3, -99.0f);
    PQUEUE_int_update(&pqi, 6, -99.001f);
    PQUEUE_int_update(&pqi, 8, -11.9f);
    size_t pqisize = pqi.size;
    success = 1;
    for (size_t i = 0; i < pqisize; i++)
        success &= PQUEUE_int_pop(&pqi) == verifyarr[i];
    EZTEST(success, "Update priority queue");
    PQUEUE_int_clear(&pqi);
	EZTEST(before_eo_tests == EZ_ALLOCATED(), "EasyObjects memory leak");
	
	// easythreads tests
	int sum = 0;
	EZ_MUTEX mutex;
	EZ_CREATE_MUTEX(mutex);
	Params par = { &sum, 10, 59, mutex };
	EZ_THREAD safe_thread;
	EZ_CREATE_THREAD(safe_thread, thread_function, &par);
	EZ_WAIT_THREAD(safe_thread);
	EZTEST(sum == 69, "Safe thread");
	sum = 0;
	EZ_THREAD unsafe_threads[1234];
	for (int i = 0; i < 1234; i++) {
		EZ_CREATE_THREAD(unsafe_threads[i], unsafe_function, &par);
	}
	for (int i = 0; i < 1234; i++) {
		EZ_WAIT_THREAD(unsafe_threads[i]);
	}
	EZTEST(sum == 1234, "Unsafe threads");
	
	// easynet tests
	size_t en_allocated = EZ_ALLOCATED();
	ez_Buffer* buf = EZ_GENERATE_BUFFER(1024);
	Packet packet = { -123, 'z', 69.0f, UINT32_MAX };
	EZTEST(buf->max_length == 1024, "Generate buffer max size");
	EZTEST(buf->current_length == 0, "Empty buffer");
	EZTEST(EZ_RECORD_BUFFER(buf, &packet), "Record buffer success");
	EZTEST(buf->current_length == sizeof(Packet), "Record buffer size");
	int count = 1023;
	while (buf->bytes[count] == 0) {
		count--;
	}
	EZTEST(count + 1 == sizeof(Packet), "Real buffer size");
	Packet affirm = { 0 };
	EZTEST(EZ_TRANSLATE_BUFFER(buf, &affirm), "Translate buffer success");
	EZTEST(affirm.a == -123 && affirm.b == 'z' && affirm.c == 69.0f && affirm.d == UINT32_MAX, "Translate buffer data");
	EZTEST(EZ_INIT_NETWORK(), "Initialize network");
	ez_Server* server = EZ_GENERATE_SERVER();
	ez_Client* client = EZ_GENERATE_CLIENT();
	ez_Ipv4 address = {{127, 0, 0, 1}};
	ez_Buffer* retbuf = EZ_GENERATE_BUFFER(1024);
	EZTEST(EZ_OPEN_SERVER(server, 55000), "Open server");
	EZTEST(EZ_CONNECT_CLIENT(client, address, 55000), "Connect client");
	ez_Connection* connection = EZ_SERVER_ACCEPT(server);
	EZTEST(connection != NULL, "Accepted client");
	EZTEST(!EZ_SERVER_ASK(connection, retbuf), "Non blocking server recieve failure");
	EZTEST(EZ_CLIENT_SEND(client, buf), "Client send");
	EZTEST(EZ_SERVER_ASK(connection, retbuf), "Non blocking server recieve success");
	EZTEST(memcmp(buf->bytes, retbuf->bytes, buf->max_length) == 0, "Successful byte transfer 1");
	memset(retbuf->bytes, 0, retbuf->max_length);
	EZ_CLIENT_SEND(client, buf);
	EZTEST(EZ_SERVER_RECIEVE(connection, retbuf), "Blocking server recieve");
	EZTEST(memcmp(buf->bytes, retbuf->bytes, buf->max_length) == 0, "Successful byte transfer 2");
	memset(retbuf->bytes, 0, retbuf->max_length);
	EZTEST(!EZ_CLIENT_ASK(client, retbuf), "Non blocking client recieve failure");
	EZTEST(EZ_SERVER_SEND(connection, buf), "Server send");
	EZTEST(EZ_CLIENT_ASK(client, retbuf), "Non blocking client recieve success");
	EZTEST(memcmp(buf->bytes, retbuf->bytes, buf->max_length) == 0, "Successful byte transfer 3");
	memset(retbuf->bytes, 0, retbuf->max_length);
	EZ_SERVER_SEND(connection, buf);
	EZTEST(EZ_CLIENT_RECIEVE(client, retbuf), "Blocking client recieve");
	EZTEST(memcmp(buf->bytes, retbuf->bytes, buf->max_length) == 0, "Successful byte transfer 4");
	EZTEST(EZ_CLOSE_CONNECTION(connection), "Close connection");
	EZTEST(EZ_DISCONNECT_CLIENT(client), "Disconnect client");
	EZTEST(EZ_CLOSE_SERVER(server), "Close server");
	EZTEST(EZ_CLEAN_NETWORK(), "Clean network");
	EZ_CLEAN_BUFFER(buf);
	EZ_CLEAN_BUFFER(retbuf);
	EZTEST(en_allocated == EZ_ALLOCATED(), "EasyNet memory leak");

	// easymath tests
	EZTEST(EZ_CLAMP(-99, -60, 100) == -60, "EasyMath clamp to min");
	EZTEST(EZ_CLAMP(999.0, 0.0, 100.0) == 100.0, "EasyMath clamp to max");
	EZTEST(EZ_CLAMP(100.0f, 0.0f, 200.0f) == 100.0f, "EasyMath clamp to none");
	EZTEST(EZ_DISTANCE(0, 25, 0, 0) == 25.0f, "EasyMath distance 1D");
	EZTEST(EZ_DISTANCE(3, 3, -3, -3) == (float)sqrt(72), "EasyMath distance square");
	EZTEST(EZ_DISTANCE(1, 10, 3, 4) == (float)sqrt(40), "EasyMath distance 2D");

    // easysort tests
    size_t before_es_tests = EZ_ALLOCATED();
    ARRLIST_int slist = { 0 };
    ARRLIST_int_add(&slist, 1);
    ARRLIST_int_add(&slist, 3);
    ARRLIST_int_add(&slist, 5);
    ARRLIST_int_add(&slist, 7);
    ARRLIST_int_add(&slist, 9);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 5, "EasySort already sorted size");
    success = slist.data[0]==1 && slist.data[1]==3 && slist.data[2]==5 && slist.data[3]==7 && slist.data[4]==9;
    EZTEST(success, "EasySort already sorted order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 1);
    ARRLIST_int_add(&slist, 3);
    ARRLIST_int_add(&slist, 2);
    ARRLIST_int_add(&slist, 5);
    ARRLIST_int_add(&slist, 7);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 5, "EasySort single dip size");
    success = 1;
    for (size_t i = 1; i < slist.size; i++)
        success &= slist.data[i] >= slist.data[i - 1];
    EZTEST(success, "EasySort single dip order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 9);
    ARRLIST_int_add(&slist, 7);
    ARRLIST_int_add(&slist, 5);
    ARRLIST_int_add(&slist, 3);
    ARRLIST_int_add(&slist, 1);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 5, "EasySort reversed size");
    success = slist.data[0]==1 && slist.data[1]==3 && slist.data[2]==5
           && slist.data[3]==7 && slist.data[4]==9;
    EZTEST(success, "EasySort reversed order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 42);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 1, "EasySort single element size");
    EZTEST(slist.data[0] == 42, "EasySort single element value");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 1);
    ARRLIST_int_add(&slist, 2);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 2, "EasySort two elements sorted size");
    EZTEST(slist.data[0]==1 && slist.data[1]==2, "EasySort two elements sorted order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 2);
    ARRLIST_int_add(&slist, 1);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 2, "EasySort two elements reversed size");
    EZTEST(slist.data[0]==1 && slist.data[1]==2, "EasySort two elements reversed order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 4);
    ARRLIST_int_add(&slist, 4);
    ARRLIST_int_add(&slist, 4);
    ARRLIST_int_add(&slist, 4);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 4, "EasySort all equal size");
    success = slist.data[0]==4 && slist.data[1]==4 && slist.data[2]==4 && slist.data[3]==4;
    EZTEST(success, "EasySort all equal values");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 1);
    ARRLIST_int_add(&slist, 5);
    ARRLIST_int_add(&slist, 3);
    ARRLIST_int_add(&slist, 5);
    ARRLIST_int_add(&slist, 5);
    ARRLIST_int_add(&slist, 9);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 6, "EasySort duplicates size");
    success = 1;
    for (size_t i = 1; i < slist.size; i++)
        success &= slist.data[i] >= slist.data[i - 1];
    EZTEST(success, "EasySort duplicates order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, -10);
    ARRLIST_int_add(&slist, -3);
    ARRLIST_int_add(&slist, -7);
    ARRLIST_int_add(&slist, 0);
    ARRLIST_int_add(&slist, 5);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 5, "EasySort negatives size");
    success = 1;
    for (size_t i = 1; i < slist.size; i++)
        success &= slist.data[i] >= slist.data[i - 1];
    EZTEST(success, "EasySort negatives order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 1);
    ARRLIST_int_add(&slist, 2);
    ARRLIST_int_add(&slist, 3);
    ARRLIST_int_add(&slist, 4);
    ARRLIST_int_add(&slist, 5);
    EasySort_int(&slist, int_score_neg);
    EZTEST(slist.size == 5, "EasySort inverted score size");
    success = 1;
    for (size_t i = 1; i < slist.size; i++)
        success &= int_score_neg(slist.data[i]) >= int_score_neg(slist.data[i - 1]);
    EZTEST(success, "EasySort inverted score order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 1);
    ARRLIST_int_add(&slist, 5);
    ARRLIST_int_add(&slist, 2);
    ARRLIST_int_add(&slist, 8);
    ARRLIST_int_add(&slist, 3);
    ARRLIST_int_add(&slist, 10);
    ARRLIST_int_add(&slist, 4);
    ARRLIST_int_add(&slist, 12);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 8, "EasySort multiple dips size");
    success = 1;
    for (size_t i = 1; i < slist.size; i++)
        success &= slist.data[i] >= slist.data[i - 1];
    EZTEST(success, "EasySort multiple dips order");
    ARRLIST_int_clear(&slist);
    ARRLIST_int_add(&slist, 99);
    ARRLIST_int_add(&slist, 1);
    ARRLIST_int_add(&slist, 2);
    ARRLIST_int_add(&slist, 3);
    ARRLIST_int_add(&slist, 4);
    EasySort_int(&slist, int_score);
    EZTEST(slist.size == 5, "EasySort high first element size");
    success = 1;
    for (size_t i = 1; i < slist.size; i++)
        success &= slist.data[i] >= slist.data[i - 1];
    EZTEST(success, "EasySort high first element order");
    ARRLIST_int_clear(&slist);
    EZTEST(before_es_tests == EZ_ALLOCATED(), "EasySort memory leak");

    // easyfile tests
    size_t before_ef_tests = EZ_ALLOCATED();
    EZTEST(ez_get_filetype("model.obj") == DOTOBJ, "EasyFile filetype obj");
    EZTEST(ez_get_filetype("model.OBJ") == DOTOBJ, "EasyFile filetype obj uppercase");
    EZTEST(ez_get_filetype("scene.prism") == DOTPRISM, "EasyFile filetype prism");
    EZTEST(ez_get_filetype("shader.spv") == DOTSPV, "EasyFile filetype spv");
    EZTEST(ez_get_filetype("material.mtl") == DOTMTL, "EasyFile filetype mtl");
    EZTEST(ez_get_filetype("data.xml") == DOTXML, "EasyFile filetype xml");
    EZTEST(ez_get_filetype("mesh.fbx") == DOTFBX, "EasyFile filetype fbx");
    EZTEST(ez_get_filetype("readme.txt") == UNKNOWN, "EasyFile filetype unknown extension");
    EZTEST(ez_get_filetype("archive.tar.gz") == UNKNOWN, "EasyFile filetype uses last extension only");
    EZTEST(ez_get_filetype("/home/user/assets/model.obj") == DOTOBJ, "EasyFile filetype with unix directory");
    EZTEST(ez_get_filetype("C:\\assets\\model.obj") == DOTOBJ, "EasyFile filetype with windows directory");
    EZTEST(ez_get_filetype("assets/model.obj/notes.xml") == DOTXML, "EasyFile filetype dot in directory name");
    EZTEST(strcmp(ez_strip_filename("/home/user/model.obj"), "model.obj") == 0, "EasyFile strip filename unix path");
    EZTEST(strcmp(ez_strip_filename("C:\\Users\\name\\model.obj"), "model.obj") == 0, "EasyFile strip filename windows path");
    EZTEST(ez_strip_filename("model.obj") == NULL, "EasyFile strip filename no directory");
    EZTEST(ez_strip_filename("/home/user/") == NULL, "EasyFile strip filename trailing slash");
    const char* ef_contents = "line one\nline two\nline three";
    FILE* ef_fixture = fopen("ef_test_fixture.xml", "wb");
    fwrite(ef_contents, 1, strlen(ef_contents), ef_fixture);
    fclose(ef_fixture);
    ez_File* ef_loaded = ez_load_file("ef_test_fixture.xml");
    EZTEST(ef_loaded != NULL, "EasyFile load existing file");
    EZTEST(ef_loaded->type == DOTXML, "EasyFile load detects filetype");
    EZTEST(ef_loaded->size == strlen(ef_contents), "EasyFile load correct size");
    EZTEST(memcmp(ef_loaded->data, ef_contents, ef_loaded->size) == 0, "EasyFile load correct data");
    FILE* ef_empty_fixture = fopen("ef_test_empty.xml", "wb");
    fclose(ef_empty_fixture);
    ez_File* ef_empty = ez_load_file("ef_test_empty.xml");
    EZTEST(ef_empty != NULL, "EasyFile load empty file succeeds");
    EZTEST(ef_empty->size == 0, "EasyFile load empty file size");
    ez_free_file(ef_empty);
    remove("ef_test_empty.xml");
    ez_FileParser ef_parser = ez_parser(ef_loaded);
    EZTEST(ef_parser.line == 0, "EasyFile parser initial line");
    EZTEST(ef_parser.cursor == 0, "EasyFile parser initial cursor");
    char ef_linebuf[64];
    EZTEST(ez_next_line(&ef_parser, ef_linebuf, sizeof(ef_linebuf)), "EasyFile next line 1 success");
    EZTEST(strcmp(ef_linebuf, "line one") == 0, "EasyFile next line 1 content");
    EZTEST(ef_parser.line == 1, "EasyFile next line 1 count");
    EZTEST(ez_next_line(&ef_parser, ef_linebuf, sizeof(ef_linebuf)), "EasyFile next line 2 success");
    EZTEST(strcmp(ef_linebuf, "line two") == 0, "EasyFile next line 2 content");
    EZTEST(ez_next_line(&ef_parser, ef_linebuf, sizeof(ef_linebuf)), "EasyFile next line 3 success");
    EZTEST(strcmp(ef_linebuf, "line three") == 0, "EasyFile next line 3 content (no trailing newline)");
    EZTEST(!ez_next_line(&ef_parser, ef_linebuf, sizeof(ef_linebuf)), "EasyFile next line exhausted");
    ez_free_file(ef_loaded);
    remove("ef_test_fixture.xml");
    
    // easyparse tests
    size_t before_ep_tests = EZ_ALLOCATED();
    float ep_float = 0.0f;
    EZTEST(ez_parse_float("3.14", &ep_float), "EasyParse float valid decimal");
    EZTEST(ep_float == 3.14f, "EasyParse float valid decimal value");
    EZTEST(ez_parse_float("-42.5", &ep_float), "EasyParse float valid negative");
    EZTEST(ep_float == -42.5f, "EasyParse float valid negative value");
    EZTEST(ez_parse_float("42", &ep_float), "EasyParse float integer string");
    EZTEST(ep_float == 42.0f, "EasyParse float integer string value");
    EZTEST(ez_parse_float("1e3", &ep_float), "EasyParse float scientific notation");
    EZTEST(ep_float == 1000.0f, "EasyParse float scientific notation value");
    EZTEST(ez_parse_float("  7.5", &ep_float), "EasyParse float leading whitespace");
    EZTEST(ep_float == 7.5f, "EasyParse float leading whitespace value");
    EZTEST(ez_parse_float("7.5  ", &ep_float), "EasyParse float trailing whitespace");
    EZTEST(ep_float == 7.5f, "EasyParse float trailing whitespace value");
    EZTEST(!ez_parse_float("abc", &ep_float), "EasyParse float invalid string");
    EZTEST(!ez_parse_float("12abc", &ep_float), "EasyParse float trailing garbage");
    EZTEST(!ez_parse_float("", &ep_float), "EasyParse float empty string");
    EZTEST(!ez_parse_float("1e400", &ep_float), "EasyParse float out of range");
    EZTEST(!ez_parse_float(NULL, &ep_float), "EasyParse float null string");
    EZTEST(!ez_parse_float("3.14", NULL), "EasyParse float null value pointer");
    size_t ep_size = 0;
    EZTEST(ez_parse_size("123", &ep_size), "EasyParse size valid");
    EZTEST(ep_size == 123, "EasyParse size valid value");
    EZTEST(ez_parse_size("0", &ep_size), "EasyParse size zero");
    EZTEST(ep_size == 0, "EasyParse size zero value");
    EZTEST(ez_parse_size("456  ", &ep_size), "EasyParse size trailing whitespace");
    EZTEST(ep_size == 456, "EasyParse size trailing whitespace value");
    EZTEST(!ez_parse_size("abc", &ep_size), "EasyParse size invalid string");
    EZTEST(!ez_parse_size("123x", &ep_size), "EasyParse size trailing garbage");
    EZTEST(!ez_parse_size("", &ep_size), "EasyParse size empty string");
    EZTEST(!ez_parse_size(NULL, &ep_size), "EasyParse size null string");
    EZTEST(!ez_parse_size("123", NULL), "EasyParse size null value pointer");
    EZTEST(ez_parse_size("-5", &ep_size), "EasyParse size negative wraps rather than failing");
    EZTEST(ep_size == (size_t)-5, "EasyParse size negative wraparound value");
    BOOL ep_bool = FALSE;
    EZTEST(ez_parse_bool("true", &ep_bool) && ep_bool == TRUE, "EasyParse bool lowercase true");
    EZTEST(ez_parse_bool("TRUE", &ep_bool) && ep_bool == TRUE, "EasyParse bool uppercase true");
    EZTEST(ez_parse_bool("True", &ep_bool) && ep_bool == TRUE, "EasyParse bool capitalized true");
    EZTEST(ez_parse_bool("false", &ep_bool) && ep_bool == FALSE, "EasyParse bool lowercase false");
    EZTEST(ez_parse_bool("FALSE", &ep_bool) && ep_bool == FALSE, "EasyParse bool uppercase false");
    EZTEST(ez_parse_bool("False", &ep_bool) && ep_bool == FALSE, "EasyParse bool capitalized false");
    EZTEST(!ez_parse_bool("tRue", &ep_bool), "EasyParse bool unsupported casing rejected");
    EZTEST(!ez_parse_bool("1", &ep_bool), "EasyParse bool invalid string rejected");
    EZTEST(!ez_parse_bool("yes", &ep_bool), "EasyParse bool non-bool word rejected");
    char* ep_argv1[] = { "hello", "world" };
    char* ep_cmd1 = ez_reconstruct_command(ep_argv1, 2);
    EZTEST(strcmp(ep_cmd1, "hello world") == 0, "EasyParse reconstruct command two args");
    EZ_FREE(ep_cmd1);
    char* ep_argv2[] = { "solo" };
    char* ep_cmd2 = ez_reconstruct_command(ep_argv2, 1);
    EZTEST(strcmp(ep_cmd2, "solo") == 0, "EasyParse reconstruct command single arg");
    EZ_FREE(ep_cmd2);
    char* ep_argv3[] = { "run", "-flag", "value", "--other" };
    char* ep_cmd3 = ez_reconstruct_command(ep_argv3, 4);
    EZTEST(strcmp(ep_cmd3, "run -flag value --other") == 0, "EasyParse reconstruct command multiple args");
    EZ_FREE(ep_cmd3);
    EZTEST(before_ep_tests == EZ_ALLOCATED(), "EasyParse memory leak");
 
    // easyprofile tests
    ez_Profiler prof = { 0 };
    ez_configure_profile(&prof, "test_profiler", 1);
    EZTEST(strcmp(prof.name, "test_profiler") == 0, "EasyProfile configure sets name");
    EZTEST(prof.step == 1, "EasyProfile configure sets step");
    ez_begin_profile(&prof);
    ez_end_profile(&prof);
    EZTEST(prof.curr >= 0.0, "EasyProfile elapsed time non-negative");
    EZTEST(prof.average == (float)prof.datastream[0], "EasyProfile single-step average matches sample");
    EZTEST(ez_profile_result(&prof) == prof.average, "EasyProfile result returns average");
    ez_Profiler prof2 = { 0 };
    ez_configure_profile(&prof2, "multi_profiler", 4);
    EZTEST(strcmp(prof2.name, "multi_profiler") == 0, "EasyProfile configure multi-step name");
    EZTEST(prof2.step == 4, "EasyProfile configure multi-step step");
    for (int ep_i = 0; ep_i < 4; ep_i++) {
        ez_begin_profile(&prof2);
        ez_sleep_ms(2);
        ez_end_profile(&prof2);
    }
    double ep_sum = 0.0;
    for (size_t ep_i = 0; ep_i < prof2.step; ep_i++) ep_sum += prof2.datastream[ep_i];
    float ep_expected_avg = (float)(ep_sum / (double)prof2.step);
    EZTEST(fabsf(prof2.average - ep_expected_avg) < 0.01f, "EasyProfile multi-sample average correctness");
    ez_Profiler prof3 = { 0 };
    ez_configure_profile(&prof3, "shift_profiler", 2);
    ez_begin_profile(&prof3);
    ez_end_profile(&prof3);
    double ep_first_sample = prof3.datastream[0];
    ez_begin_profile(&prof3);
    ez_sleep_ms(50);
    ez_end_profile(&prof3);
    EZTEST(prof3.datastream[1] == ep_first_sample, "EasyProfile shifts older sample back");
    EZTEST(prof3.datastream[0] > prof3.datastream[1], "EasyProfile newest sample lands at front");

    EZTEST(before_ef_tests == EZ_ALLOCATED(), "EasyFile memory leak");
	printf("\nTest suite results: %s%d%s/%d tests passed\n", 
		passed_tests == total_tests ? EZ_GREEN : EZ_RED, passed_tests, EZ_RESET, total_tests);
    if (passed_tests != total_tests) return -1;
	return 0;
}
