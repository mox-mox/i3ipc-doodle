#pragma once

template<typename _Tp>
class fixed_array
{
	public:
	typedef _Tp 	    			              value_type;
	typedef value_type*			                  pointer;
	typedef const value_type*                     const_pointer;
	typedef value_type&                   	      reference;
	typedef const value_type&             	      const_reference;
	typedef value_type*          		          iterator;
	typedef const value_type*			          const_iterator;
	typedef std::size_t                    	      size_type;
	typedef std::ptrdiff_t                   	  difference_type;
	typedef std::reverse_iterator<iterator>	      reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

	private:
	const std::size_t _size;
	pointer _data;


	public:
	//{{{
	fixed_array(std::size_t size) :
		_size(size),
		//_data(new value_type[size]),
		_data(reinterpret_cast<pointer>(malloc(size*sizeof(value_type))))
	{}
	//}}}

	//{{{
	~fixed_array(void)
	{
		    //delete [] _data;
			free(_data);
	}
	//}}}

	//{{{ The normal functions

	//{{{ Element access.

	constexpr pointer data() noexcept
	{
		return _data;
	}

	constexpr const_pointer data() const noexcept
	{
		return _data;
	}

	constexpr reference operator[] (size_type idx) noexcept
	{
		return _data[idx];
	}

	constexpr const_reference operator[] (size_type idx) const noexcept
	{
		return _data[idx];
	}

	constexpr reference at(size_type idx)
	{
		if( idx >= _size ) throw std::out_of_range("fixed_array::at: idx (which is "+std::to_string(idx)+") >= _size (which is "+std::to_string(_size)+")");
		return _data[idx];
	}

	constexpr const_reference at(size_type idx) const
	{
		if( idx >= _size ) throw std::out_of_range("fixed_array::at: idx (which is "+std::to_string(idx)+") >= _size (which is "+std::to_string(_size)+")");
		return _data[idx];
	}

	constexpr reference front() noexcept
	{
		return *begin();
	}

	constexpr const_reference front() const noexcept
	{
		return _data;
	}

	constexpr reference back() noexcept
	{
		return _size ? *(end()-1) : *end();
	}

	constexpr const_reference back() const noexcept
	{
		return _size ? *(end()-1) : *end();
	}
	//}}}

	//{{{ Iterators.
	constexpr iterator begin() noexcept
	{
		return _data;
	}

	constexpr const_iterator begin() const noexcept
	{
		return _data;
	}

	constexpr iterator end() noexcept
	{
		return _data+_size;
	}

	constexpr const_iterator end() const noexcept
	{
		return _data+_size;
	}

	constexpr reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	constexpr const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	constexpr reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	constexpr const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	constexpr const_iterator cbegin() const noexcept
	{
		return const_iterator(_data);
	}

	constexpr const_iterator cend() const noexcept
	{
		return const_iterator(_data+_size);
	}

	constexpr const_reverse_iterator crbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	constexpr const_reverse_iterator crend() const noexcept
	{
		return const_reverse_iterator(begin());
	}
	//}}}

	//{{{ Capacity.
	constexpr size_type size() const noexcept
	{
		return _size;
	}

	constexpr size_type max_size() const noexcept
	{
		return _size;
	}

	constexpr bool empty() const noexcept
	{
		return size() == 0;
	}
	//}}}

	// DR 776.
	void fill(const value_type& __u)
	{
		std::fill_n(begin(), size(), __u);
	}
	//}}}

	//{{{ Emplace by iterator or index

	template<typename... _Args>
	iterator emplace(iterator __position, _Args&&... __args)
	{
		new (__position) value_type(std::forward<_Args>(__args)...);
		return __position;
	}

	template<typename... _Args>
	iterator emplace(size_type idx, _Args&&... __args)
	{
		if( idx >= _size ) throw std::out_of_range("fixed_array::at: idx (which is "+std::to_string(idx)+") >= _size (which is "+std::to_string(_size)+")");
		new (_data+idx) value_type(std::forward<_Args>(__args)...);
		return _data+idx;
	}
	//}}}
};
