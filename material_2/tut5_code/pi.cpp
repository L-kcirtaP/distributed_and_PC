#include <cstdio>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cassert>
using std::cout;
using std::endl;
using std::ifstream;
using std::ostringstream;
using std::string;
using std::vector;

#define SCALE 10000
#define ARRINIT 2000

void pi_series_algorithm(long digits, string &pi_)
{
    digits = (digits / 4) * 14;

    ostringstream pi;
    pi.fill('0');
    vector<long> arr(digits + 1);

    for (int i = 0; i <= digits; ++i)
    {
        arr[i] = ARRINIT;
    }

    long sum = 0;
    int bug_at = 0;

    for (int i = digits; i > 0; i -= 14)
    {
        int carry = sum % SCALE;
        sum = 0;

        for (int j = i; j > 0; --j)
        {
            sum = sum * j + SCALE * arr[j];
            assert(sum >= 0); /* overflow check */
            arr[j] = sum % ((j << 1) - 1);
            assert(arr[j] >= 0); /* overflow check */
            sum /= (j << 1) - 1;
        }

        bug_at += 4;

        if (!(carry + sum / SCALE < 10000 && carry + sum / SCALE >= 0))
        {
            printf("Bug found! carry, sum, carry + sum / SCALE, digit: %d %ld %d %ld %d\n",
                   carry, sum, SCALE, carry + sum / SCALE, bug_at);
        }

        //assert(carry + sum / SCALE < 10000 && carry + sum / SCALE >= 0);
        pi << std::setw(4) << std::right << carry + sum / SCALE;
    }

    pi_ = pi.str();
}

void read_pi_digits(long nd, string &pi_)
{
    ifstream pi_file("pi-0001.txt");
    /* http://www.piday.org/
       http://piworld.calico.jp/estart.html
       https://crypto.stanford.edu/pbc/notes/pi/code.html
     */
    ostringstream pi;

    for (int l = 0; l < nd / 100; ++l)
    {
        string line;
        getline(pi_file, line);
        string::iterator it = line.begin();

        for (int b = 0; b < 10; ++b)
        {
            for (int c = 0; c < 10; ++c, ++it)
            {
                pi << *it;
            }

            ++it;
        }
    }

    pi_file.close();

    pi_ = pi.str();
}

int main()
{
    printf("Pi string\t%s\n",
           "3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117067982148");

    long nd = 200; // Number of digits
    // long nd = 54940; // This fails
    /* Algorithm fails at digit 54,935 (when processing digits 54,936 to 54,940) */

    // Read digits from file
    string pi_digits;
    read_pi_digits(nd, pi_digits);
    printf("Pi file\t\t  %.100s\n", pi_digits.c_str());

    // Compute digits using series
    string pi_series;
    pi_series_algorithm(nd, pi_series);
    printf("Series\t\t %.100s\n", pi_series.c_str());

    int d_debug = std::min(pi_digits.size(), pi_series.size() - 1);
    for (int i = 0; i < d_debug; ++i)
    {
        if (!(pi_digits[i] == pi_series[i + 1]))
        {
            cout << "Error in digit #" << i + 1 << endl;
        }
        assert(pi_digits[i] == pi_series[i + 1]);
    }

#if 0
    // Debugging; testing where the calculation fails
    // Check
    int ib = 54936-4;
    int ie = 54936+8;

    for(int i=ib; i<ie; ++i) {
        cout << pi_digits[i-1];
    }

    cout << endl;

    for(int i=ib; i<ie; ++i) {
        cout << pi_series[i];
    }

    cout << endl;
#endif

    return 0;
}
