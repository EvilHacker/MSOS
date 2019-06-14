#ifndef _AddressTranslationTableBuilder_h_
#define _AddressTranslationTableBuilder_h_

#include "../cPrimitiveTypes.h"

//------------------------------------------------------------------------------------------------
// * class AddressTranslationTableBuilder
//
// Creates level 1 and level 2 translation tables for the StrongARM MMU.
// The MMU must be disabled or, alternatively, the translation tables must be located in a
// virtual = physical address space.
// Because this code will most likely run from within its load region, there should be no
// external references code or data in an execution region.
// Global variables should not be referenced. External functions should not be called.
//------------------------------------------------------------------------------------------------

class AddressTranslationTableBuilder
{
public:
	// constructors
	AddressTranslationTableBuilder(void *pTableStart, Bool growsUpwards);
	AddressTranslationTableBuilder(
		void *pTableBase,
		void *pTableLimit,
		Bool growsUpwards);

	// accessing
	inline void *getTableBase();
	inline void *getTableLimit();
	inline void *getL1Table();

	// address space mapping
	enum AccessPermission
	{
		noSupervisorAccessPermission,
		noAccessPermission,
		readOnlyAccessPermission,
		readWriteAccessPermission
	};
	inline void map(
		const void *pVirtual,
		const void *pPhysical,
		UInt size,
		UInt domain,
		AccessPermission accessPermission,
		Bool cachable,
		Bool bufferable);
	void map(
		UInt virtualAddress,
		UInt physicalAddress,
		UInt size,
		UInt domain,
		AccessPermission accessPermission,
		Bool cachable,
		Bool bufferable);
	inline void unmap(
		const void *pVirtual,
		UInt size);
	inline void unmap(
		UInt virtualAddress,
		UInt size);

private:
	// types
	typedef UInt TableEntry;
	typedef TableEntry L1TableEntry;
	typedef TableEntry L2TableEntry;

	// table parameters
	static const UInt sectionSize = 0x100000;
	static const UInt largePageSize = 0x10000;
	static const UInt smallPageSize = 0x1000;

	// address space mapping
	void mapSection(
		UInt virtualAddress,
		UInt physicalAddress,
		UInt domain,
		AccessPermission accessPermission,
		Bool cachable,
		Bool bufferable);
	void mapLargePage(
		UInt virtualAddress,
		UInt physicalAddress,
		UInt domain,
		AccessPermission accessPermission,
		Bool cachable,
		Bool bufferable);
	void mapSmallPage(
		UInt virtualAddress,
		UInt physicalAddress,
		UInt domain,
		AccessPermission accessPermission,
		Bool cachable,
		Bool bufferable);
	void mapQuarterSmallPage(
		UInt virtualAddress,
		UInt physicalAddress,
		UInt domain,
		AccessPermission accessPermission,
		Bool cachable,
		Bool bufferable);
	L2TableEntry *mapL2Table(UInt virtualAddress, UInt domain);

	// translation table allocation
	TableEntry *allocateTable(UInt sizeInBytes);

	// representation
	void *pTableBase;
	void *pTableLimit;
	L1TableEntry *pL1Table;
	Bool growsUpwards;
};

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::getTableBase
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void *AddressTranslationTableBuilder::getTableBase()
{
	return pTableBase;
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::getTableLimit
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void *AddressTranslationTableBuilder::getTableLimit()
{
	return pTableLimit;
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::getL1Table
//
// Accessor.
//------------------------------------------------------------------------------------------------

inline void *AddressTranslationTableBuilder::getL1Table()
{
	return pL1Table;
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::map
//
// Maps a <pVirtual> address space to a <pPhysical> address space.
//------------------------------------------------------------------------------------------------

inline void AddressTranslationTableBuilder::map(
	const void *pVirtual,
	const void *pPhysical,
	UInt size,
	UInt domain,
	AccessPermission accessPermission,
	Bool cachable,
	Bool bufferable)
{
	map((UInt)pVirtual, (UInt)pPhysical, size, domain, accessPermission, cachable, bufferable);
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::unmap
//
// Unmaps a <pVirtual> address space.
//------------------------------------------------------------------------------------------------

inline void AddressTranslationTableBuilder::unmap(
	const void *pVirtual,
	UInt size)
{
	map((UInt)pVirtual, 0, size, 0, noSupervisorAccessPermission, false, false);
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::unmap
//
// Unmaps a <pVirtual> address space.
//------------------------------------------------------------------------------------------------

inline void AddressTranslationTableBuilder::unmap(
	UInt virtualAddress,
	UInt size)
{
	map(virtualAddress, 0, size, 0, noSupervisorAccessPermission, false, false);
}

#endif // _AddressTranslationTableBuilder_h_
