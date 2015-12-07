#include <iostream>

#include "gpiowork.h"

using namespace std;

int main()
{
	cout << "Hello World!" << endl;

	gpio::gpiowork wrk;

	boost::thread thr(wrk);

	wrk.open_pin(1, 1.5, 50);
	wrk.open_pin(2, 1.1, 500);

	thr.join();

	return 0;
}

