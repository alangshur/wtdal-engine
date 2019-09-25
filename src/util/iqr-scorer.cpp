#include <iostream>
#include <algorithm>
#include "util/iqr-scorer.hpp"
using namespace std;

IQRScorer::IQRScorer() : sample_sum(0), sample_count(0) {
    this->q1 = { 0, false, this->first_quartile_set.begin(), 
        this->first_quartile_set.end() };
    this->median = { 0, false, 0, 0 };
    this->q3 = { 0, false, this->third_quartile_set.begin(), 
        this->third_quartile_set.end() };
    srand(time(NULL));
}

void IQRScorer::add_sample(uint32_t sample) {

    // initiate scorer
    if (this->sample_count == 0) {
        this->q1 = { double(sample), false, this->first_quartile_set.begin(), 
            this->first_quartile_set.end() };
        this->median = { double(sample), false, sample, 0 };
        this->q3 = { double(sample), false, this->third_quartile_set.begin(), 
            this->third_quartile_set.end() };
    }

    // handle single-sample case
    else if (this->sample_count == 1) {
        uint32_t val_f = sample < this->median.value_f ? sample : this->median.value_f;
        uint32_t val_s = sample < this->median.value_f ? this->median.value_f : sample;
        auto it_f = this->first_quartile_set.insert(val_f);
        auto it_s = this->third_quartile_set.insert(val_s);

        // update quartiles
        this->q1 = { double(val_f), false, it_f, this->first_quartile_set.end() };
        this->median = { double(val_f + val_s) / 2.0, true, val_f, val_s };
        this->q3 = { double(val_s), false, it_s, this->third_quartile_set.end() };
    }

    // handle variable-sample case
    else {

        // add to composite median
        if (this->median.is_composite_quartile) {
            if (sample < this->median.value_f) {
                this->add_quartile(sample, this->q1, this->first_quartile_set);
                uint32_t balance_sample = *(this->first_quartile_set.rbegin());
                this->median = { double(balance_sample), false, balance_sample, 0 };
                this->remove_quartile(balance_sample, this->q1, this->first_quartile_set);
            }
            else if (sample >= this->median.value_s) {
                this->add_quartile(sample, this->q3, this->third_quartile_set);
                uint32_t balance_sample = *(--this->third_quartile_set.rend());
                this->median = { double(balance_sample), false, balance_sample, 0 };
                this->remove_quartile(balance_sample, this->q3, this->third_quartile_set);
            }
            else this->median = { double(sample), false, sample, 0 };
        }

        // add to single-item median
        else {
            if (sample >= this->median.value_f) {
                this->add_quartile(sample, q3, this->third_quartile_set);
                uint32_t balance_sample = *(--this->third_quartile_set.rend());
                uint32_t balance_median = this->median.value_f;
                this->median = { 
                    (double(balance_median) + double(balance_sample)) / 2.0,
                    true, balance_median, balance_sample
                };
                this->add_quartile(balance_median, q1, this->first_quartile_set);
            }
            else {
                this->add_quartile(sample, q1, this->first_quartile_set);
                uint32_t balance_sample = *(this->first_quartile_set.rbegin());
                uint32_t balance_median = this->median.value_f;
                this->median = { 
                    (double(balance_sample) + double(balance_median)) / 2.0,
                    true, balance_sample, balance_median
                };
                this->add_quartile(balance_median, q3, this->third_quartile_set);
            }
        }
    }

    // increment counters
    this->sample_sum += sample;
    this->sample_count++;
}

void IQRScorer::remove_sample(uint32_t sample) {
    if (!this->sample_count)
        throw runtime_error("No samples to be removed");

    // de-initiate scorer
    if (this->sample_count == 1) {
        this->q1 = { 0, false, this->first_quartile_set.begin(), 
            this->first_quartile_set.end() };
        this->median = { 0, false, 0, 0 };
        this->q3 = { 0, false, this->third_quartile_set.begin(), 
            this->third_quartile_set.end() };
    }

    // handle single-sample case
    else if (this->sample_count == 2) {
        if ((this->median.value_f != sample) && (this->median.value_s != sample))
            throw runtime_error("Cannot find sample to be removed");
        uint32_t balance_median = this->median.value_f == sample ? 
            this->median.value_s : this->median.value_f;
        this->first_quartile_set.clear();
        this->third_quartile_set.clear();

        // update quartiles
        this->q1 = { double(balance_median), false, this->first_quartile_set.begin(), 
            this->first_quartile_set.end() };
        this->median = { double(balance_median), false, balance_median, 0 };
        this->q3 = { double(balance_median), false, this->third_quartile_set.begin(), 
            this->third_quartile_set.end() };
    }

    // handle variable-sample case
    else {

        // remove from composite median
        if (this->median.is_composite_quartile) {
            if (sample <= this->median.value_f) {
                this->remove_quartile(sample, q1, this->first_quartile_set);
                uint32_t balance_sample = this->median.value_s;
                this->median = { double(balance_sample), false, balance_sample, 0 };
                this->remove_quartile(balance_sample, q3, this->third_quartile_set);
            }
            else if (sample >= this->median.value_s) {
                this->remove_quartile(sample, q3, this->third_quartile_set);
                uint32_t balance_sample = this->median.value_f;
                this->median = { double(balance_sample), false, balance_sample, 0 };
                this->remove_quartile(balance_sample, q1, this->first_quartile_set);
            }
            else throw runtime_error("Cannot find sample to be removed");
        }

        // remove from single-item median
        else {
            if (sample > this->median.value_f) {
                this->remove_quartile(sample, q3, third_quartile_set);
                this->add_quartile(this->median.value_f, q3, third_quartile_set);
                uint32_t balance_sample = *(this->first_quartile_set.rbegin());
                this->median = { 
                    double(balance_sample + this->median.value_f) / 2.0, true,
                    balance_sample, this->median.value_f
                };
            }
            else if (sample < this->median.value_f) {
                this->remove_quartile(sample, q1, first_quartile_set);
                this->add_quartile(this->median.value_f, q1, first_quartile_set);
                uint32_t balance_sample = *(--this->third_quartile_set.rend());
                this->median = { 
                    double(this->median.value_f + balance_sample) / 2.0, true,
                    this->median.value_f, balance_sample
                };
            }
            else if (sample == this->median.value_f) {
                uint32_t balance_sample_q1 = *(this->first_quartile_set.rbegin());
                uint32_t balance_sample_q3 = *(--this->third_quartile_set.rend());
                this->median = { 
                    double(balance_sample_q1 + balance_sample_q3) / 2.0, true,
                    balance_sample_q1, balance_sample_q3
                };
            }
            else throw runtime_error("Cannot find sample to be removed");
        }
    }

    // decrement counters
    this->sample_sum -= sample;
    this->sample_count--;
}

double IQRScorer::get_iqr() { return this->get_q3() - this->get_q1(); }
double IQRScorer::get_q1() { return (this->q1).value; }
double IQRScorer::get_median() { return (this->median).value; }
double IQRScorer::get_q3() { return (this->q3).value; }
uint32_t IQRScorer::size() { return this->sample_count; }
double IQRScorer::get_mean() {
    if (!this->sample_count)
        throw runtime_error("Cannot find sample to be removed");
    return double(this->sample_sum) / double(this->sample_count); 
}

enum outlier_type IQRScorer::is_outlier(double value) {
    if (value > this->get_q3() + (OUTLIER_COEFF * this->get_iqr())) return Above;
    else if (value < this->get_q1() - (OUTLIER_COEFF * this->get_iqr())) return Below;
    else return No;
}

uint32_t IQRScorer::fetch_random_sample() {
    if (!this->sample_count)
        throw runtime_error("Cannot find sample to be removed");

    // account for non-composite median
    if (!this->median.is_composite_quartile && 
        !(rand() % this->sample_count)) return this->median.value_f;
 
    // execute composite pipeline
    else {
        multiset<uint32_t>::iterator it;
        uint32_t it_jump = rand() % (this->sample_count / 2);    
        if (rand() % 2) it = begin(this->first_quartile_set);
        else it = begin(this->third_quartile_set);
        advance(it, it_jump);
        return *it;
    }
}

uint32_t IQRScorer::fetch_random_sample_linear() {
    if (!this->sample_count)
        throw runtime_error("Cannot find sample to be removed");

    // select random number linearly 
    uint32_t rand_n_1 = rand() % this->sample_count, 
        rand_n_2 = rand() % this->sample_count;
    uint32_t rand_n = ((rand_n_1 > rand_n_2) ? rand_n_1 : rand_n_2);

    // account for non-composite median
    if (!this->median.is_composite_quartile && 
        (rand_n == (this->sample_count / 2))) 
        return this->median.value_f;

    // execute composite pipeline
    else {
        uint32_t it_jump;
        multiset<uint32_t>::iterator it;
        if (rand_n < (this->sample_count / 2)) {
            it = begin(this->first_quartile_set);
            it_jump = rand_n;
        }
        else {
            it = begin(this->third_quartile_set);
            it_jump = rand_n - (this->sample_count / 2) 
                - !this->median.is_composite_quartile;
        }

        // advance iterator
        advance(it, it_jump);
        return *it;
    }
}

void IQRScorer::print_quartile_set() {
    if (!this->sample_count) return;
    if (this->sample_count == 1) {
        cout << this->median.value_f << endl << flush;
        return;
    }

    // print first quartile
    auto curr_it = this->first_quartile_set.begin();
    cout << *curr_it;
    while (++curr_it != this->first_quartile_set.end())
        cout << " - " << *curr_it;

    // print median
    if (!this->median.is_composite_quartile) 
        cout << " - " << this->median.value_f;

    // print third quartile
    curr_it = this->third_quartile_set.begin();
    while (curr_it != this->third_quartile_set.end())
        cout << " - " << *curr_it++;
    cout << endl << flush;
}   

void IQRScorer::add_even_quartile_it(quartile_t& quartile, 
    multiset<uint32_t>& quartile_set, bool less_than_quartile) {

    // handle quartile-sample placement
    if (less_than_quartile) {
        quartile = {
            double(*prev(quartile.it_f) + *(quartile.it_f)) / 2.0,
            true,
            prev(quartile.it_f),
            quartile.it_f
        };
    }
    else {
        quartile = {
            double(*(quartile.it_f) + *next(quartile.it_f)) / 2.0,
            true,
            quartile.it_f,
            next(quartile.it_f)
        };
    }
}

void IQRScorer::add_odd_quartile_it(quartile_t& quartile, 
    multiset<uint32_t>& quartile_set, bool less_than_quartile) {

    // handle quartile-sample placement
    if (less_than_quartile) {
        quartile = {
            double(*(quartile.it_f)),
            false,
            quartile.it_f,
            quartile_set.end()
        };
    }
    else {
        quartile = {
            double(*next(quartile.it_f)),
            false,
            next(quartile.it_f),
            quartile_set.end()
        };
    }
}

void IQRScorer::add_quartile(uint32_t sample, quartile_t& quartile,
    multiset<uint32_t>& quartile_set) {
    quartile_set.insert(sample);

    // update specified set quartile 
    if (quartile_set.size() % 2) {
        if (double(sample) >= *(quartile.it_f)) 
            this->add_odd_quartile_it(quartile, quartile_set, false);
        else this->add_odd_quartile_it(quartile, quartile_set, true);
    }
    else {
        if (double(sample) >= *(quartile.it_f)) 
            this->add_even_quartile_it(quartile, quartile_set, false);
        else this->add_even_quartile_it(quartile, quartile_set, true);
    }
}

void IQRScorer::remove_even_quartile_it(quartile_t& quartile, 
    multiset<uint32_t>& quartile_set, bool less_than_quartile,
    bool is_first_it) {

    // handle quartile-sample placement
    if (is_first_it) {
        quartile = {
            double(*prev(quartile.it_f) + *next(quartile.it_f)) / 2.0,
            true,
            prev(quartile.it_f),
            next(quartile.it_f)
        };
    }
    else if (less_than_quartile) {
        quartile = {
            double(*(quartile.it_f) + *next(quartile.it_f)) / 2.0,
            true,
            quartile.it_f,
            next(quartile.it_f)
        };
    }
    else {
        quartile = {
            double(*prev(quartile.it_f) + *(quartile.it_f)) / 2.0,
            true,
            prev(quartile.it_f),
            quartile.it_f
        };
    }
}

void IQRScorer::remove_odd_quartile_it(quartile_t& quartile, 
    multiset<uint32_t>& quartile_set, bool less_than_quartile,
    bool is_first_it) {

    // handle quartile-sample placement
    if (is_first_it || less_than_quartile) {
        quartile = {
            double(*(quartile.it_s)),
            false,
            quartile.it_s,
            quartile_set.end()
        };
    }
    else {
        quartile = {
            double(*(quartile.it_f)),
            false,
            quartile.it_f,
            quartile_set.end()
        };
    }
}

void IQRScorer::remove_quartile(uint32_t sample, quartile_t& quartile,
    multiset<uint32_t>& quartile_set) {

    // find sample iterator
    auto it = quartile_set.find(sample);
    if (it == quartile_set.end()) 
        throw runtime_error("Cannot find sample to be removed");
    bool is_first_it = it == quartile.it_f;

    // update specified set quartile 
    if ((quartile_set.size() - 1) % 2) {
        if (double(sample) >= *(quartile.it_f)) 
            this->remove_odd_quartile_it(quartile, quartile_set, false, is_first_it);
        else this->remove_odd_quartile_it(quartile, quartile_set, true, is_first_it);
    }
    else {
        if (double(sample) >= *(quartile.it_f)) 
            this->remove_even_quartile_it(quartile, quartile_set, false, is_first_it);
        else this->remove_even_quartile_it(quartile, quartile_set, true, is_first_it);
    }

    // erase sample
    quartile_set.erase(it);
}