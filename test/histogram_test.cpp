#include "utils/histogram/chrono_axes.hpp"
#include "test.hpp"
#include <boost/histogram.hpp>
#include <boost/histogram/ostream.hpp>
#include <set>
#include <iterator>
#include <sstream>
#include <string>
#include <chrono>

BOOST_AUTO_TEST_SUITE(histogram_test_suite)

BOOST_AUTO_TEST_CASE(regular_time_histogram_test) {
   auto h = boost::histogram::make_histogram(histogram::axis::duration<std::chrono::duration<double, std::nano> /*std::chrono::nanoseconds*/,
                                                                       boost::histogram::axis::regular<double,
                                                                                                       boost::histogram::use_default,
                                                                                                       boost::histogram::use_default,
                                                                                                       // Create a 1d-histogram with a regular axis that has 5 equidistant bins on the real line from - 1.0 to 7000000
                                                                                                       boost::histogram::axis::option::growth_t>>{5, -1., 7000000});
   BOOST_CHECK_EQUAL(std::distance(h.cbegin(), h.cend()), 5);
   h(std::chrono::milliseconds{1});
   h(std::chrono::nanoseconds{22});
   BOOST_CHECK_EQUAL(std::distance(h.cbegin(), h.cend()), 5);
   h(std::chrono::nanoseconds{9000000});
   BOOST_CHECK_EQUAL(std::distance(h.cbegin(), h.cend()), 7);
   h(std::chrono::nanoseconds{190000000000});

   auto n = std::distance(h.cbegin(), h.cend());
   BOOST_CHECK_EQUAL(n, 135715);
   std::ostringstream out;
   dump(out, h);
   auto str = out.str();
   BOOST_CHECK(!str.empty());
}

BOOST_AUTO_TEST_CASE(chrono_axes_variable_histogram_test) {
   auto h = boost::histogram::make_histogram(histogram::axis::duration<std::chrono::duration<double, std::nano> /*std::chrono::nanoseconds*/,
                                                                       boost::histogram::axis::variable<double,
                                                                                                        boost::histogram::use_default,
                                                                                                        boost::histogram::axis::option::growth_t>>{1, 20, 100, 110, 150});
   h(std::chrono::milliseconds{1});
   h(std::chrono::nanoseconds{22});
   h(std::chrono::nanoseconds{9000000});
   auto n = std::distance(h.cbegin(), h.cend());
   BOOST_CHECK_EQUAL(n, 6);
}

BOOST_AUTO_TEST_CASE(variable_histogram_test) {
   auto buckets = std::set<double>{1, 20, 100, 110, 150};
   auto h = boost::histogram::make_histogram(boost::histogram::axis::variable<double,
                                                                              boost::histogram::use_default,
                                                                              boost::histogram::axis::option::growth_t>(buckets.begin(), buckets.end())
                                             //        1, 20, 100, 110, 150
   );

   h(1);
   h(220);
   h(9000000);
   auto n = std::distance(h.cbegin(), h.cend());
   BOOST_CHECK_EQUAL(n, 6);
}

BOOST_AUTO_TEST_CASE(int_histogram_test) {
   auto h = boost::histogram::make_histogram(boost::histogram::axis::integer<int,
                                                                             boost::histogram::use_default,
                                                                             boost::histogram::axis::option::growth_t>{/*2, 0., 1.*/});

   h(1);
   h(22);
   h(9000000);
   auto n = std::distance(h.cbegin(), h.cend());
   BOOST_CHECK_EQUAL(n, 9000001);
}

BOOST_AUTO_TEST_CASE(regular_histogram_test) {
   auto h = boost::histogram::make_histogram(boost::histogram::axis::regular<double,
                                                                             boost::histogram::use_default,
                                                                             boost::histogram::use_default,
                                                                             boost::histogram::axis::option::growth_t>{/*2, 0., 1.*/});

   h(1);
   h(22);
   h(10000);
   auto n = std::distance(h.cbegin(), h.cend());
   BOOST_CHECK_EQUAL(n, 1);
}

BOOST_AUTO_TEST_CASE(custom_growing_test) {
   class axis2d_growing {
    public:
      auto index(std::tuple<double, double> xy) const {
         const auto x = std::get<0>(xy);
         const auto y = std::get<1>(xy);
         const auto r = std::sqrt(x * x + y * y);
         return std::min(static_cast<boost::histogram::axis::index_type>(r), size());
      }

      auto update(std::tuple<double, double> xy) {
         const auto x = std::get<0>(xy);
         const auto y = std::get<1>(xy);
         const auto r = std::sqrt(x * x + y * y);
         const auto n = static_cast<int>(r);
         const auto old = size_;
         if (n >= size_) size_ = n + 1;
         return std::make_pair(n, old - size_);
      }

      boost::histogram::axis::index_type size() const { return size_; }

    private:
      boost::histogram::axis::index_type size_ = 0;
   };

   auto h = boost::histogram::make_histogram_with(std::vector<int>{}, axis2d_growing{});

   h(1, 0);
   h(0, 22);
   h(101, 1);
   auto n = std::distance(h.cbegin(), h.cend());
   BOOST_CHECK_EQUAL(n, 102);
}
BOOST_AUTO_TEST_SUITE_END()
