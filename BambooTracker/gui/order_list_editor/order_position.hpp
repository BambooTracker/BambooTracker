#ifndef ORDER_POSITION_HPP
#define ORDER_POSITION_HPP

struct OrderPosition
{
	int trackVisIdx, row;

	friend bool operator==(const OrderPosition& a, const OrderPosition& b);
	friend bool operator!=(const OrderPosition& a, const OrderPosition& b);
};

inline bool operator==(const OrderPosition& a, const OrderPosition& b)
{
	return (a.trackVisIdx == b.trackVisIdx && a.row == b.row);
}

inline bool operator!=(const OrderPosition& a, const OrderPosition& b)
{
	return !(a == b);
}

#endif // ORDER_POSITION_HPP
