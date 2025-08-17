module;
#include <algorithm>
#include <iterator>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>
export module utility:container;

// Concept for smart pointers
template <typename T>
struct s_is_smart_pointer : std::false_type
{
};

template <typename T>
struct s_is_smart_pointer<std::shared_ptr<T>> : std::true_type
{
};

template <typename T>
struct s_is_smart_pointer<std::unique_ptr<T>> : std::true_type
{
};

template <typename T>
struct s_is_smart_pointer<std::weak_ptr<T>> : std::true_type
{
};

template <typename T>
concept smart_pointer = s_is_smart_pointer<std::remove_cvref_t<T>>::value;

export namespace utility
{

    /**
     * @brief A list of smart pointers that support automatic memory management
     * Has smart iterators looping only through the valid elements, along with other utility functions.
     *
     * @tparam Type - The type of the object to store in the list
     * @tparam Pointer - Type of the smart pointer to use, defaults to std::shared_ptr
     * @tparam Container - Type of the container to use, defaults to std::vector
     */
    template <typename Type, smart_pointer Pointer = std::shared_ptr<Type>, typename Container = std::vector<Pointer>>
        requires std::ranges::bidirectional_range<Container>
    class c_container
    {
    public:
        using value_type = Type;
        using pointer_type = Pointer;
        using container_type = Container;

        c_container() = default;

        c_container(const c_container &) = default;
        c_container &operator=(const c_container &) = default;

        c_container(c_container &&) noexcept = default;
        c_container &operator=(c_container &&) noexcept = default;

        ~c_container() = default;

        /**
         * Custom iterator for smart iteration.
         *
         * Implementation of a bidirectional iterator that iterates over valid elements, skipping over invalid pointers automacally
         *
         * @param T - Type of the object to store in the list
         * @param is_const - Whether the iterator is const or not
         * @param is_reverse - Whether the iterator is reverse or not
         */
        template <bool is_const = false, bool is_reverse = false>
        class iterator
        {
        public:
            using iterator_category = std::bidirectional_iterator_tag;
            using value_type = typename Container::value_type;
            using difference_type = typename Container::difference_type;
            using pointer = std::conditional_t<is_const, const value_type *, value_type *>;
            using reference = std::conditional_t<is_const, const value_type &, value_type &>;

            iterator(typename Container::iterator iter, typename Container::iterator begin, typename Container::iterator end)
                : m_it(iter), m_begin(begin), m_end(end)
            {
                if constexpr (!is_reverse)
                {
                    skip_invalid_forward();
                }
                else
                {
                    skip_invalid_backward();
                }
            }

            iterator(const iterator &) = default;
            iterator(iterator &&) = default;
            iterator &operator=(const iterator &) = default;
            iterator &operator=(iterator &&) = default;

            iterator &operator++();
            iterator operator++(int);
            iterator &operator--();
            iterator operator--(int);

            reference operator*()
            {
                return *m_it;
            }

            pointer operator->()
            {
                return &(*m_it);
            }

            bool operator==(const iterator &other) const
            {
                return m_it == other.m_it;
            }

            bool operator!=(const iterator &other) const
            {
                return m_it != other.m_it;
            }

        private:
            using underlying_iterator = std::conditional_t<is_const, typename Container::const_iterator, typename Container::iterator>;

            underlying_iterator m_it;
            underlying_iterator m_begin;
            underlying_iterator m_end;

            void skip_invalid_forward();
            void skip_invalid_backward();
            bool is_valid_pointer(const pointer_type &ptr);
        };

        void push(pointer_type &&item);
        void push(const pointer_type &item);

        void pop();
        void clear();

        [[nodiscard]] std::size_t size() const noexcept;
        [[nodiscard]] std::size_t size_valid() const noexcept;
        [[nodiscard]] bool empty() const noexcept;

        [[nodiscard]] iterator<> begin() noexcept;
        [[nodiscard]] iterator<> end() noexcept;

        [[nodiscard]] iterator<true> begin() const noexcept;
        [[nodiscard]] iterator<true> end() const noexcept;

        [[nodiscard]] iterator<false, true> rbegin() noexcept;
        [[nodiscard]] iterator<false, true> rend() noexcept;

        [[nodiscard]] iterator<true, true> rbegin() const noexcept;
        [[nodiscard]] iterator<true, true> rend() const noexcept;

    private:
        Container m_container;
    };

} // namespace utility

// Implementation
namespace utility
{
    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    void c_container<Type, Pointer, Container>::push(pointer_type &&item)
    {
        m_container.push_back(std::move(item));
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    void c_container<Type, Pointer, Container>::push(const pointer_type &item)
    {
        m_container.push_back(item);
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    void c_container<Type, Pointer, Container>::pop()
    {
        while (!m_container.empty() && !iterator<>::is_valid_pointer(m_container.back()))
        {
            m_container.pop_back();
        }
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    void c_container<Type, Pointer, Container>::clear()
    {
        m_container.clear();
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    std::size_t c_container<Type, Pointer, Container>::size() const noexcept
    {
        return m_container.size();
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    std::size_t c_container<Type, Pointer, Container>::size_valid() const noexcept
    {
        return std::ranges::count_if(m_container, [](const pointer_type &ptr)
                                     { return iterator<>::is_valid_pointer(ptr); });
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    bool c_container<Type, Pointer, Container>::empty() const noexcept
    {
        return m_container.empty();
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<> c_container<Type, Pointer, Container>::begin() noexcept
    {
        return iterator(m_container.beegin(), m_container.begin(), m_container.end());
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<> c_container<Type, Pointer, Container>::end() noexcept
    {
        return iterator(m_container.end(), m_container.begin(), m_container.end());
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<true> c_container<Type, Pointer, Container>::begin() const noexcept
    {
        return iterator(m_container.begin(), m_container.begin(), m_container.end());
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<true> c_container<Type, Pointer, Container>::end() const noexcept
    {
        return iterator(m_container.end(), m_container.begin(), m_container.end());
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<false, true> c_container<Type, Pointer, Container>::rbegin() noexcept
    {
        return iterator(m_container.rbegin(), m_container.rbegin(), m_container.rend());
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<false, true> c_container<Type, Pointer, Container>::rend() noexcept
    {
        return iterator(m_container.rend(), m_container.rbegin(), m_container.rend());
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<true, true> c_container<Type, Pointer, Container>::rbegin() const noexcept
    {
        return iterator(m_container.rbegin(), m_container.rbegin(), m_container.rend());
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    typename c_container<Type, Pointer, Container>::template iterator<true, true> c_container<Type, Pointer, Container>::rend() const noexcept
    {
        return iterator(m_container.rend(), m_container.rbegin(), m_container.rend());
    }

    // Iterator implementations
    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    template <bool is_const, bool is_reverse>
    bool c_container<Type, Pointer, Container>::iterator<is_const, is_reverse>::is_valid_pointer(const pointer_type &ptr)
    {
        if constexpr (std::is_same_v<pointer_type, std::shared_ptr<Type>> or std::is_same_v<pointer_type, std::unique_ptr<Type>>)
        {
            return ptr != nullptr;
        }
        else if constexpr (std::is_same_v<pointer_type, std::weak_ptr<Type>>)
        {
            return !ptr.expired();
        }
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    template <bool is_const, bool is_reverse>
    void c_container<Type, Pointer, Container>::iterator<is_const, is_reverse>::skip_invalid_forward()
    {
        while (m_it != m_end && !is_valid_pointer(*m_it))
        {
            ++m_it;
        }
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    template <bool is_const, bool is_reverse>
    void c_container<Type, Pointer, Container>::iterator<is_const, is_reverse>::skip_invalid_backward()
    {
        while (m_it != m_begin && !is_valid_pointer(*m_it))
        {
            --m_it;
        }
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    template <bool is_const, bool is_reverse>
    typename c_container<Type, Pointer, Container>::template iterator<is_const, is_reverse> &c_container<Type, Pointer, Container>::iterator<is_const, is_reverse>::operator++()
    {
        if (m_it != m_end)
        {
            ++m_it;
            skip_invalid_forward();
        }
        return *this;
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    template <bool is_const, bool is_reverse>
    typename c_container<Type, Pointer, Container>::template iterator<is_const, is_reverse> c_container<Type, Pointer, Container>::iterator<is_const, is_reverse>::operator++(int)
    {
        iterator tmp = *this;
        ++(*this);
        return tmp;
    }

    template <typename Type, smart_pointer Pointer, typename Container>
        requires std::ranges::bidirectional_range<Container>
    template <bool is_const, bool is_reverse>
    typename c_container<Type, Pointer, Container>::template iterator<is_const, is_reverse> &c_container<Type, Pointer, Container>::iterator<is_const, is_reverse>::operator--()
    {
        if (m_it != m_begin)
        {
            --m_it;
            skip_invalid_backward();
        }
        return *this;
    }

} // namespace utility
