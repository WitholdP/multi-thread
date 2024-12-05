#include <iostream>
#include <vector>
#include <map>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

using namespace std;

class Production
{
public:
	void PlaceOrder(const string &id)
	{
		unique_lock<mutex> lock(mu);
		orders.push_back(id);
		orderStatus[id] = false;
		OrderReady.notify_one();
		printf("Order has been placed %s\n", id.c_str());
	}

	void Produce()
	{
		while (true)
		{

			unique_lock<mutex> lock(mu);
			OrderReady.wait(lock, [this]()
							{ return !orders.empty() || stopProduction; });

			if (stopProduction && orders.empty())
				break;

			this_thread::sleep_for(std::chrono::seconds(1));
			string productionId = orders.back();
			orderStatus[productionId] = true;
			orders.pop_back();
			printf("Order has been produced %s\n", productionId.c_str());
		}
	}

	void PrintOrders()
	{
		for (auto const &[key, val] : orderStatus)
		{
			printf("%s : %s \n", key.c_str(), val ? "Produced" : "Not produced");
		}
	}

	void StopProduction()
	{
		unique_lock<mutex> lock(mu);
		stopProduction = true;
		OrderReady.notify_all();
	}

private:
	mutex mu;
	condition_variable OrderReady;
	map<string, bool> orderStatus;
	vector<string> orders;
	bool stopProduction = false;
};

int main()
{
	Production production;

	thread orderThread([&production]()
					   {
                           production.PlaceOrder("Order1");
                           production.PlaceOrder("Order2");
                           production.PlaceOrder("Order3"); });

	thread produceThread(&Production::Produce, &production);

	orderThread.join();

	production.StopProduction();

	produceThread.join();

	production.PrintOrders();

	return 0;
};
