#ifndef IQR_SCORER_H
#define IQR_SCORER_H

#include <cstdint>
#include <utility>
#include <set>

const float OUTLIER_COEFF = 1.5;

enum outlier_type {
    No = 0,
    Below = 1,
    Above = 2,
};

typedef struct {
    double value;
    bool is_composite_quartile;
    std::multiset<uint32_t>::iterator it_f;
    std::multiset<uint32_t>::iterator it_s;
} quartile_t;

typedef struct {
    double value;
    bool is_composite_quartile;
    uint32_t value_f;
    uint32_t value_s;
} median_quartile_t;

/*
    The IQRScorer class is used to execute dynamic
    statistical studies on a set of continously
    inserted and removed studies. These include mean,
    median, Q1, Q3, IQR, and outlier measurements, as 
    well as random sample generation.
*/
class IQRScorer {
    public:
        IQRScorer();
        void add_sample(uint32_t sample);
        void remove_sample(uint32_t sample);
        double get_iqr();
        double get_q1();
        double get_median();
        double get_q3();
        double get_mean();
        uint32_t size();
        enum outlier_type is_outlier(double value);
        uint32_t fetch_random_sample();
        void print_quartile_set();

    private:
        void add_even_quartile_it(quartile_t& quartile, 
            std::multiset<uint32_t>& quartile_set, bool less_than_quartile);
        void add_odd_quartile_it(quartile_t& quartile, 
            std::multiset<uint32_t>& quartile_set, bool less_than_quartile);
        void add_quartile(uint32_t sample, quartile_t& quartile,
            std::multiset<uint32_t>& quartile_set);
        void remove_even_quartile_it(quartile_t& quartile, 
            std::multiset<uint32_t>& quartile_set, bool less_than_quartile,
            bool is_first_it);
        void remove_odd_quartile_it(quartile_t& quartile, 
            std::multiset<uint32_t>& quartile_set, bool less_than_quartile,
            bool is_first_it);
        void remove_quartile(uint32_t sample, quartile_t& quartile,
            std::multiset<uint32_t>& quartile_set);

        quartile_t q1, q3;
        std::multiset<uint32_t> first_quartile_set;
        std::multiset<uint32_t> third_quartile_set;
        median_quartile_t median;
        uint32_t sample_sum;
        uint32_t sample_count;
};

#endif