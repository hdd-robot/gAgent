/*
 * DataTime.hpp

 */

#ifndef DATETIME_HPP_
#define DATETIME_HPP_

namespace gagent {

class DataTime {
public:
	DataTime();
	virtual ~DataTime();
private:

	// IDL struct to represent a time stamp.
	// It is based on the ISO8601 format with extension for millisecond durations.
	// The value of the typeDesignator must be a valid
	// AlphaCharacter, i.e. ['a'-'z' , 'A'-'Z'], that identifies the timezone.
	// ISO8601 reports the mapping between typeDesignator and timezone.
	// The typeDesignator for UTC is the character 'Z'.
	// If the value of typeDesignator is not an AlphaCharacter, it defaults
	// to the local timezone.

	short year;  			// year (e.g. 2000)
	short month; 			// between 1 and 12
	short day;   			// between 1 and 31
	short hour;  			// between 0 and 23
	short minutes; 			// between 0 and 59
	short seconds; 			// between 0 and 59
	short milliseconds; 	// between 0 and 999
	char  typeDesignator; 	// see comment above

};

} /* namespace gagent */

#else
namespace gagent {
	class DataTime;
}
#endif /* DATETIME_HPP_ */
