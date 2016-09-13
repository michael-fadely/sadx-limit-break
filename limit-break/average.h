#pragma once

template <typename T> class Average
{
private:
	T* points;
	int size;
	int current_size;
	bool invalid;
	double average;

public:
	explicit Average(int size)
	{
		reset();
		this->size = size;
		this->points = new T[size];
	}

	~Average()
	{
		delete points;
	}

	bool add_point(T point)
	{
		invalid = true;

		if (current_size)
		{
			for (auto i = min(size - 1, current_size); i > 0; i--)
			{
				if (i == size)
					throw;
				points[i] = points[i - 1];
			}
		}

		points[0] = point;

		auto result = ready();

		if (current_size < size)
			++current_size;

		return result;
	}

	double get_average()
	{
		if (invalid)
		{
			double result = 0.0;
			auto _count = (double)current_size;

			for (auto i = 0; i < _count; i++)
				result += points[i];

			average = result / _count;
			invalid = false;
		}

		return average;
	}

	bool ready() const
	{
		return current_size == size;
	}

	void reset()
	{
		invalid = true;
		current_size = 0;
	}
};
