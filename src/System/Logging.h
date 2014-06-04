#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>

#define PRINT_LEVEL 5

/**
 * For printing debug messages - enabled for developing only.
 */
#if PRINT_LEVEL >= 5
#define PRINT_DEBUG
#endif

/**
 * Tons of messages concerning sending and receiving.
 */
#if PRINT_LEVEL >= 4
#define PRINT_LOG
#endif

/**
 * Information about state changes.
 */
#if PRINT_LEVEL >= 3
#define PRINT_INFO
#endif

/**
 * When something went wrong.
 */
#if PRINT_LEVEL >= 2
#define PRINT_WARNING
#endif

/**
 * When something dangerous happened and this is all about this program when it happens.
 */
#if PRINT_LEVEL >= 1
#define PRINT_ERROR
#endif

namespace EM {
	class NullStream : public std::ostream {
	public:
		NullStream() : std::ios(0), std::ostream(0) {}
		NullStream(NullStream &&) : std::ios(0), std::ostream(0) {}
	};

	extern NullStream nullstream;
}

inline std::ostream &debug()
{
#ifdef PRINT_DEBUG
	return std::cerr;
#else
	return EM::nullstream;
#endif
}

inline std::ostream &log()
{
#ifdef PRINT_LOG
	return std::cerr;
#else
	return EM::nullstream;
#endif
}

inline std::ostream &info()
{
#ifdef PRINT_INFO
	return std::cerr;
#else
	return EM::nullstream;
#endif
}

inline std::ostream &warn()
{
#ifdef PRINT_WARNING
	return std::cerr;
#else
	return EM::nullstream;
#endif
}

inline std::ostream &error()
{
#ifdef PRINT_ERROR
	return std::cerr;
#else
	return EM::nullstream;
#endif
}

#endif // LOGGING_H