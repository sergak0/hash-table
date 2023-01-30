/*
 * Realization of Robin Hood hash table (open addressing).
 *
 * The algorithm is based on the notion of probe sequence lengths (PSL). The PSL of a key
 * is the number of probes required to find the key during lookup.
 *
 * A key with a low PSL can be thought of as rich, and a key with a high PSL can be thought of as poor.
 * When inserting a new key the algorithm moves the rich in favor
 * of the poor (“takes from the rich and gives to the poor”), hence the name Robin Hood hashing.
 */

#include <vector>
#include <stdexcept>
#include "list"


template<class KeyType, class ValueType, class Hash = std::hash<KeyType> > class HashMap{

public:
    using KvType = std::pair<KeyType, ValueType>;
    // define hash map element
    struct Node {
        KvType keyvalue;
        int status;
        size_t dist_to_ideal;

        Node() {
            status = -1;
            dist_to_ideal = 0;
        }

        explicit Node(KvType p, size_t dist) {
            keyvalue = p;
            status = 1;
            dist_to_ideal = dist;
        }
    };

    // define hash map iterators
    template <typename ContT, typename IterVal> struct hm_iterator {
        explicit hm_iterator() : hm_(nullptr) {}

        explicit hm_iterator(ContT *hm) : hm_(hm) { advance_past_empty(); }

        explicit hm_iterator(ContT *hm, size_t idx) : hm_(hm), idx_(idx) {}

        template <typename OtherContT, typename OtherIterVal>
        explicit hm_iterator(const hm_iterator<OtherContT, OtherIterVal> &other): hm_(other.hm_), idx_(other.idx_) {}

        bool operator==(const hm_iterator &other) const {
            return other.hm_ == hm_ && other.idx_ == idx_;
        }
        bool operator!=(const hm_iterator &other) const {
            return !(other == *this);
        }

        hm_iterator &operator++() {
            ++idx_;
            advance_past_empty();
            return *this;
        }

        hm_iterator operator++(int) {
            auto tmp = *this;
            ++idx_;
            advance_past_empty();
            return tmp;
        }
        

        IterVal& operator*() {
            return *get_cur_pointer();
        }

        IterVal* operator->() {
              return get_cur_pointer();
        }

    private:
        void advance_past_empty() {
            while (idx_ < hm_->data_.size() && hm_->data_[idx_].status != 1) {
                ++idx_;
            }
        }

        auto get_cur_pointer() {
            // casting std::pair<KeyType, ValueType> to std::pair<const KeyType, ValueType>
            return reinterpret_cast<IterVal*> (&(hm_->data_[idx_].keyvalue));
        }

        ContT *hm_ = nullptr;
        size_t idx_ = 0;
        friend ContT;
    };

    using iterator = hm_iterator<HashMap, std::pair<const KeyType, ValueType>>;
    using const_iterator = hm_iterator<const HashMap, const std::pair<const KeyType, ValueType>>;
    
    // constructors
    explicit HashMap(Hash hasher_ = Hash()): hasher_(std::move(hasher_)){
        buffer_size_ = default_size_;
        cnt_all_ = 0;
        cnt_dead_ = 0;
        data_.resize(buffer_size_);
    }

    HashMap(const HashMap& other){
        *this = other;
    }
    
    HashMap& operator = (const HashMap& other) {
        hasher_ = other.hasher_;
        cnt_all_ = other.cnt_all_;
        cnt_dead_ = other.cnt_dead_;
        buffer_size_ = other.buffer_size_;
        data_ = other.data_;
        return *this;
    }

    HashMap(KvType* start, KvType* end) : HashMap(){
        KvType* cur = start;
        while(cur != end){
            insert(*cur);
            cur++;
        }
    }

    HashMap(iterator begin, iterator end): HashMap(){
        iterator cur = begin;
        while(cur != end){
            insert(*cur);
            cur++;
        }
    }

    HashMap(const_iterator begin, const_iterator end): HashMap(){
        const_iterator cur = begin;
        while(cur != end){
            insert(*cur);
            cur++;
        }
    }

    HashMap(std::initializer_list<KvType> list): HashMap(){
        auto cur = list.begin();
        while(cur != list.end()){
            insert(*cur);
            cur++;
        }
    }

    // simple functions
    size_t size() const {
        return cnt_all_ - cnt_dead_;
    }

    bool empty() const {
        return cnt_all_ - cnt_dead_ == 0;
    }

    Hash hash_function() const {
        return hasher_;
    }

    iterator begin() {
        return iterator(this);
    }

    const_iterator begin() const {
        return const_iterator(this);
    }

    iterator end() {
        return iterator(this, data_.size());
    }

    const_iterator end() const {
        return const_iterator(this, data_.size());
    }

    // not such simple functions
    iterator insert(KvType keyvalue) {
        resize();
        KeyType key = keyvalue.first;
        size_t h1 = get_hash(key);

        size_t cur_dist_to_ideal = 0;
        int insert_index = -1;
        for (size_t i = 0; i < buffer_size_; i++, cur_dist_to_ideal++){
            size_t index = (h1 + i) % buffer_size_;
            if (data_[index].status == 1 && data_[index].keyvalue.first == key) {
                return iterator(this, index);
            }

            // balancing Rich and Poor elements
            if (data_[index].status == 1 && data_[index].dist_to_ideal < cur_dist_to_ideal) {
                std::swap(data_[index].keyvalue, keyvalue);
                std::swap(data_[index].dist_to_ideal, cur_dist_to_ideal);
                if (insert_index == -1) {
                    insert_index = static_cast<int>(index);
                }
            }

            if (data_[index].status == -1) {
                data_[index] = Node(keyvalue, cur_dist_to_ideal);
                cnt_all_++;
                if (insert_index == -1) {
                    insert_index = static_cast<int>(index);
                }
                return iterator(this, insert_index);
            }
        }
        return iterator(this, buffer_size_);
    }

    void erase(const KeyType& key) {
        size_t h1 = get_hash(key);

        for(size_t i = 0; i < buffer_size_; i++){
            size_t index = (h1 + i) % buffer_size_;
            if (data_[index].status == 1 && data_[index].keyvalue.first == key) {
                data_[index].status = 0;
                cnt_dead_++;
            }
            if (data_[index].status == -1) {
                return;
            }
        }
    }

    iterator find(const KeyType& key) {
        size_t h1 = get_hash(key);
        for(size_t i = 0; i < buffer_size_; i++){
            size_t index = (h1 + i) % buffer_size_;
            if (data_[index].status == 1 && data_[index].keyvalue.first == key) {
                return iterator(this, index);
            }
            if (data_[index].status == -1) {
                return iterator(this, buffer_size_);
            }
        }
        return iterator(this, buffer_size_);
    }

    const_iterator find(const KeyType& key) const {
        size_t h1 = get_hash(key);
        for(size_t i = 0; i < buffer_size_; i++){
            size_t index = (h1 + i) % buffer_size_;
            if (data_[index].status == 1 && data_[index].keyvalue.first == key) {
                return const_iterator(this, index);
            }
            if (data_[index].status == -1) {
                return const_iterator(this, buffer_size_);
            }
        }
        return const_iterator(this, buffer_size_);
    }

    ValueType& operator [](KeyType key){
        iterator it = find(key);
        if (it == end()){
            it = insert(std::pair<KeyType, ValueType>{key, ValueType()});
        }
        return it -> second;
    }

    const ValueType& at(const KeyType& key) const{
        const_iterator it = find(key);
        if (it == end()){
            throw std::out_of_range("very sad:(");
        }
        return it -> second;
    }

    void clear() {
        *this = HashMap<KeyType, ValueType, Hash>(hasher_);
    }
    
private:
    Hash hasher_;
    std::vector<Node> data_;
    size_t default_size_ = 16;
    const double load_factor_ = 0.5;
    size_t cnt_all_ = 0;
    size_t cnt_dead_ = 0;
    size_t buffer_size_ = 0;


    size_t get_hash(const KeyType& k) const {
        return hasher_(k) % buffer_size_;
    }

    void resize() {
        if (buffer_size_ * load_factor_ < cnt_all_) {
            buffer_size_ *= 2;
        } else {
            return;
        }

        cnt_dead_ = 0;
        cnt_all_ = 0;
        std::vector <Node> data_2(buffer_size_);
        std::swap(data_, data_2);
        for (size_t i = 0; i < data_2.size(); i++) {
            if (data_2[i].status == 1) {
                insert(data_2[i].keyvalue);
            }
        }
    }
};