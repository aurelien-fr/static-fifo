/**
 * @file FIFO.hpp
 * @author aurelien.dhiver@outlook.fr
 * 
 * Copyright (c) 2025 aurelien.dhiver@outlook.fr
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 * 
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <span>

/**
 * @brief FIFO class push and pull data from a static container.
 * This FIFO does NOT support access from concurrent threads (for now)
 */
template <typename T, size_t t_size>
class Fifo {
  public:

    /**
     * @brief Construct a new Fifo object
     */
    Fifo() : m_readIdx(0), m_writeIdx(0), m_nbElements(0) {}

    /**
     * @brief Construct a new Fifo object
     * @param[in] initializer list with the first elements of the FIFO. The number of elements shall
     * be <= to the FIFO size
     */
    Fifo(const std::initializer_list<T> &initList) : Fifo() {

        if (initList.size() > t_size) {
            assert("Too many elements to initialize FIFO");
        }

        for (const auto &item : initList) {
            push(item);
        }
    }

    /**
     * @brief Get current number of elements to be read in the FIFO buffer
     * @return Number of elements
     */
    size_t getCount() { return m_nbElements; }

    /**
     * @brief Empty the FIFO, delete all the data
     */
    void reset() {
        m_writeIdx = 0;
        m_readIdx = 0;
        m_nbElements = 0;
    }

    /**
     * @brief Read a single element from the FIFO
     * @param[out] dest Where a single byte from the FIFO is written
     * @return True if reading is done, otherwise false
     */
    bool pop_back(T *const dest) {
        auto ret = true;
        assert(dest != nullptr);

        if (m_nbElements) {
            *dest = m_buffer[m_readIdx];
            m_readIdx = (m_readIdx + 1) % t_size;
            --m_nbElements;
        } else {
            ret = false;
        }

        return ret;
    };

    /**
     * @brief Write data to the FIFO. If overwrite is not selected and there is not enough space,
     * leave the FIFO as is.
     * @param[out] src Source buffer to copy the data from
     * @param[in] overwrite Overwrite previous elements if not enough space in the FIFO
     * @return Number of elements copied in the FIFO
     */
    size_t push(std::span<const T> src, bool overwrite = false) {
        size_t nbElementsCopied = 0U;

        if ((!overwrite) && (src.size() > (t_size - m_nbElements))) {
            return 0;
        }

        for (const T &element : src) {
            m_buffer[m_writeIdx] = element;
            m_writeIdx = (m_writeIdx + 1) % t_size;

            if (m_nbElements < t_size) {
                m_nbElements++;
                nbElementsCopied++;
            } else {
                // in case of data overwrite
                m_readIdx = m_writeIdx;
            }
        }

        return nbElementsCopied;
    }

    /**
     * @brief Write data to the FIFO. If overwrite is not selected and there is not enough space,
     * leave the FIFO as is.
     * @param[out] src Source buffer to copy the data from
     * @param[in] overwrite Overwrite previous elements if not enough space in the FIFO
     * @return Number of elements copied in the FIFO
     */
    template <size_t t_arrSz>
    size_t push(const std::array<const T, t_arrSz> &src, bool overwrite = false) {
        return push(std::span<const T>{src}, overwrite);
    }

    /**
     * @brief Write data to the FIFO. If overwrite is not selected and there is not enough space,
     * leave the FIFO as is.
     * @param[out] src Initializer list containing elements to push to the FIFO
     * @param[in] overwrite Overwrite previous elements if not enough space in the FIFO
     * @return Number of elements copied in the FIFO
     */
    size_t push(const std::initializer_list<T> &src, bool overwrite = false) {
        return push(std::span<const T>{src.begin(), src.size()}, overwrite);
    }

    /**
     * @brief Write data to the FIFO. If overwrite is not selected and there is not enough space,
     * leave the FIFO as is.
     * @param[out] src Source buffer to copy the data from
     * @param[in] size Number of elements to copy to the FIFO
     * @param[in] overwrite Overwrite previous elements if not enough space in the FIFO
     * @return Number of elements copied in the FIFO
     */
    size_t push(T *src, size_t size, bool overwrite = false) {
        assert(src != nullptr);
        return push(std::span<const T>{src, size}, overwrite);
    }

    /**
     * @brief Write a single sample it the FIFO.
     * @param[in] sample to write
     * @param[in] overwrite Overwrite previous element if not enough space in the FIFO
     * @return True if the sample was written, otherwise false.
     */
    bool push(T var, bool overwrite = false) { return (push(&var, 1, overwrite) != 0); }

    /**
     * @brief Drop a number of samples of the FIFO. These samples cannot be retrieved.
     * @return Number of samples dropped
     */
    size_t drop(size_t size) {
        auto droppedSamples = std::min(m_nbElements, size);
        m_nbElements -= droppedSamples;
        m_readIdx = (m_readIdx + droppedSamples) % t_size;
        return droppedSamples;
    }

    /**
     * @brief Read data from the FIFO and delete the read data
     * @param[out] dest Destination buffer where the data are written to
     * @param[in] availSpace Available space in the destination buffer, in elements
     * @return Number of elements read
     */
    size_t pull(T *destination, size_t availSpace) {
        const auto nbElementsToCopy = availSpace;
        assert(destination != nullptr);

        while (availSpace > 0 && m_nbElements > 0) {
            *destination = m_buffer[m_readIdx];
            m_readIdx = (m_readIdx + 1) % t_size;
            destination++;
            --availSpace;
            --m_nbElements;
        }

        return (nbElementsToCopy - availSpace);
    }

    /**
     * @brief Read data from the FIFO without deleting the data from the FIFO.
     * The same values can be read several times.
     * @warning This does not delete the data from the FIFO
     * @param[out] dest Destination buffer where the data are written to
     * @param[in] availSpace Available space in the destination buffer, in elements
     * @return Number of elements read
     */
    size_t read(T *destination, size_t availSpace) {
        const auto nbElementsToCopy = availSpace;
        assert(destination != nullptr);

        auto readIdx = m_readIdx;
        auto remainInFifo = m_nbElements;

        while (availSpace > 0 && remainInFifo > 0) {
            *destination = m_buffer[readIdx];
            readIdx = (readIdx + 1) % t_size;
            destination++;
            --availSpace;
            --remainInFifo;
        }

        return (nbElementsToCopy - availSpace);
    }

    /**
     * @brief Access to a specific element in the circular buffer
     * @warning It's the caller responsability to access the right index, otherwise undefined number
     */
    T &operator[](std::size_t idx) {
        auto readIndex = (m_readIdx + idx) % t_size;
        return (m_buffer[readIndex]);
    }

    /**
     * @brief Direct accessor to the underlying data element - should not be used...
     * @return const T* Pointer to the internal data buffer
     */
    const T *data(void) { return m_buffer.data(); }

    /**
     * @brief Iterator object to simplify operations on FIFO
     */
    class Iterator {
      public:
        Iterator() = delete;
        Iterator(const Fifo<T, t_size> &fifo, size_t index, size_t remaining)
            : m_fifo(fifo), m_index(index), m_remaining(remaining) {}

        T operator*() const { return m_fifo.m_buffer[m_index]; }

        Iterator &operator++() {
            if (m_remaining > 0) {
                m_index = (m_index + 1) % t_size;
                --m_remaining;
            }
            return *this;
        }

        bool operator==(const Iterator &other) const {
            return m_index == other.m_index && m_remaining == other.m_remaining;
        }

      private:
        const Fifo<T, t_size> &m_fifo;
        size_t m_index;
        size_t m_remaining;
    };

    /// @brief begin operator of FIFO object
    Iterator begin() const { return Iterator(*this, m_readIdx, m_nbElements); }

    /// @brief end operator of FIFO object
    Iterator end() const { return Iterator(*this, (m_readIdx + m_nbElements) % t_size, 0); }

    /// @brief Operator overload for '==' operation. Only works for FIFO of same type.
    bool operator==(const Fifo<T, t_size> &other) const {
        if (m_nbElements != other.m_nbElements) {
            return false;
        }

        size_t idx = m_readIdx;
        size_t otherIdx = other.m_readIdx;

        for (size_t i = 0; i < m_nbElements; i++) {
            if (m_buffer[idx] != other.m_buffer[otherIdx]) {
                return false;
            }
            idx = (idx + 1) % t_size;
            otherIdx = (otherIdx + 1) % t_size;
        }

        return true;
    }

  private:
    /**
     * @brief Container where the FIFO elements are stored
     */
    std::array<T, t_size> m_buffer{};

    /**
     * @brief Current read index, where the next data to read is
     */
    size_t m_readIdx{0U};

    /**
     * @brief Write index, index where to write the next data
     */
    size_t m_writeIdx{0U};

    /**
     * @brief Current nb of elements in the FIFO
     */
    size_t m_nbElements{0U};
};
