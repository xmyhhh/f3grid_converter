#pragma once


template <typename T>
struct  priority_pair {
	T t;
	double priority = -1;
};

template <typename T>
class priority_queue {

public:
	priority_queue() {}

	std::vector<priority_pair<T>> queue;

	void push_back(T t, double priority) {
		queue.push_back({ t,priority });
		std::sort(queue.begin(), queue.end(), [](priority_pair<T> p1, priority_pair<T> p2) {return p1.priority < p2.priority; });

	}
	void remove_at(int index) {
		queue.erase(index);
	}

	T operator[](const int index)
	{
		return queue[index].t;

	}
	size_t size() {
		return queue.size();
	}

	double max_priority() {
		if (queue.size() > 0)
			return queue[queue.size() - 1].priority;
		else
			return -1;
	}

	double min_priority() {
		if (queue.size() > 0)
			return queue[0].priority;
		else
			return -1;
	}


	priority_pair<T> get_upper(double priority) {
		ASSERT(max_priority() > priority);

		for (int i = queue.size() - 1; i >= 0; i--) {
			if (queue[i].priority < priority)

				if (i == (queue.size() - 1))
					return queue[i];
				else
					return queue[i + 1];
		}
		return  queue[0];
	}

	priority_pair<T> get_lower(double priority) {
		ASSERT(min_priority() < priority);

		for (int i = 0; i < queue.size(); i++) {
			if (queue[i].priority > priority)
				if (i == 0)
					return queue[i];
				else
					return queue[i - 1];
		}
		return  queue[queue.size() - 1];

	}
};