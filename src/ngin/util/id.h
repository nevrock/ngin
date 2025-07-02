#ifndef ID_UTIL_HPP
#define ID_UTIL_HPP

#include <atomic> // For std::atomic to ensure thread-safe ID generation
#include <limits> // For std::numeric_limits

/**
 * @brief A header-only utility class for generating unique unsigned integer IDs.
 *
 * This class provides a simple, thread-safe mechanism to retrieve a unique
 * unsigned integer ID at any point. IDs are generated sequentially, starting
 * from 1, and increment up to the maximum value of an unsigned int.
 *
 * It uses std::atomic to ensure thread safety when multiple threads attempt
 * to retrieve IDs concurrently.
 */
class IdUtil {
public:
    /**
     * @brief Retrieves a unique unsigned integer ID.
     *
     * This method atomically increments an internal counter and returns its
     * new value. It ensures that each call returns a unique ID within the
     * lifetime of the application, up to the maximum value of an unsigned int.
     *
     * If the ID counter reaches its maximum value (std::numeric_limits<unsigned int>::max()),
     * subsequent calls will wrap around to 0. However, this scenario is highly
     * unlikely in most applications due to the vast range of unsigned int.
     *
     * @return A unique unsigned integer ID.
     */
    static unsigned int get_unique_id() {
        // Atomically increment the counter and return the new value.
        // The pre-increment ensures that the first ID returned is 1, not 0.
        // A simple ++next_id_ is atomic for std::atomic<unsigned int>
        // and returns the post-incremented value.
        return ++next_id_;
    }

private:
    // Private constructor to prevent instantiation, as this is a utility class
    // with only static methods.
    IdUtil() = delete;

    // Private destructor to prevent deletion.
    ~IdUtil() = delete;

    // Private copy constructor and assignment operator to prevent copying.
    IdUtil(const IdUtil&) = delete;
    IdUtil& operator=(const IdUtil&) = delete;

    /**
     * @brief Static atomic counter to keep track of the next available ID.
     *
     * std::atomic ensures that operations on next_id_ are thread-safe,
     * preventing race conditions when multiple threads request IDs.
     * Initialized to 0 so the first ID returned by get_unique_id() is 1.
     *
     * Using 'static inline' (C++17 and later) allows the static member
     * to be defined and initialized directly within the class definition
     * in a header file, avoiding the need for a separate out-of-class definition.
     */
    static inline std::atomic<unsigned int> next_id_ = 0;
};

// The out-of-class initialization for next_id_ is no longer needed
// due to the use of 'static inline' (C++17 and later).
// std::atomic<unsigned int> IdUtil::next_id_ = 0;

#endif // ID_UTIL_HPP
