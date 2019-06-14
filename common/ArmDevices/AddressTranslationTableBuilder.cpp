#include "AddressTranslationTableBuilder.h"
#include "../pointerArithmetic.h" // all inline functions no external code referenced

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::AddressTranslationTableBuilder
//
// Constructor.
// Creates new address translation tables.
// <pTableStart> must be aligned to a 16KB boundary.
//------------------------------------------------------------------------------------------------

AddressTranslationTableBuilder::AddressTranslationTableBuilder(
	void *pTableStart, Bool growsUpwards)
{
	this->pTableBase = pTableStart;
	this->pTableLimit = pTableStart;
	this->growsUpwards = growsUpwards;

	// initialize the level 1 translation table
	pL1Table = allocateTable(16 * 1024);
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::AddressTranslationTableBuilder
//
// Constructor.
// Opens existing address translation tables for modification.
// <pTableBase> and <pTableLimit> must point to previously constructed tables.
//------------------------------------------------------------------------------------------------

AddressTranslationTableBuilder::AddressTranslationTableBuilder(
	void *pTableBase,
	void *pTableLimit,
	Bool growsUpwards)
{
	this->pTableBase = pTableBase;
	this->pTableLimit = pTableLimit;
	this->growsUpwards = growsUpwards;

	// get the existing level 1 translation table
	if(growsUpwards)
	{
		this->pL1Table = (L1TableEntry *)pTableBase;
	}
	else
	{
		this->pL1Table = subtractFromPointer((L1TableEntry *)pTableLimit, 16 * 1024);
	}
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::map
//
// Maps an <virtualAddress> space to a <physicalAddress> space.
//------------------------------------------------------------------------------------------------

void AddressTranslationTableBuilder::map(
	UInt virtualAddress,
	UInt physicalAddress,
	UInt size,
	UInt domain,
	AccessPermission accessPermission,
	Bool cachable,
	Bool bufferable)
{
	// map the address space in pieces
	while(size > 0)
	{
		// check if a 1 megabyte section can be mapped
		if((virtualAddress & (sectionSize - 1)) == 0 && size >= sectionSize)
		{
			// map a section
			mapSection(virtualAddress, physicalAddress,
				domain, accessPermission, cachable, bufferable);

			// advance to the next piece of the address space
			virtualAddress += sectionSize;
			physicalAddress += sectionSize;
			size -= sectionSize;
			continue;
		}

		// check if a 64 kilobyte large page can be mapped
		if((virtualAddress & (largePageSize - 1)) == 0 && size >= largePageSize)
		{
			// map a large page
			mapLargePage(virtualAddress, physicalAddress,
				domain, accessPermission, cachable, bufferable);

			// advance to the next piece of the address space
			virtualAddress += largePageSize;
			physicalAddress += largePageSize;
			size -= largePageSize;
			continue;
		}

		// check if a 4 kilobyte small page can be mapped
		if((virtualAddress & (smallPageSize - 1)) == 0 && size >= smallPageSize)
		{
			// map a small page
			mapSmallPage(virtualAddress, physicalAddress,
				domain, accessPermission, cachable, bufferable);

			// advance to the next piece of the address space
			virtualAddress += smallPageSize;
			physicalAddress += smallPageSize;
			size -= smallPageSize;
			continue;
		}

		// check if a 1 kilobyte quarter of a small page can be mapped
		if((virtualAddress & (smallPageSize / 4 - 1)) == 0 && size >= smallPageSize / 4)
		{
			// map a quarter of a small page
			mapQuarterSmallPage(virtualAddress, physicalAddress,
				domain, accessPermission, cachable, bufferable);

			// advance to the next piece of the address space
			virtualAddress += smallPageSize / 4;
			physicalAddress += smallPageSize / 4;
			size -= smallPageSize / 4;
			continue;
		}

		// error, address space is missaligned or too small to map
		return;
	}
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::mapSection
//
// Maps an <virtualAddress> space to a <physicalAddress> space.
//------------------------------------------------------------------------------------------------

void AddressTranslationTableBuilder::mapSection(
	UInt virtualAddress,
	UInt physicalAddress,
	UInt domain,
	AccessPermission accessPermission,
	Bool cachable,
	Bool bufferable)
{
	pL1Table[virtualAddress >> 20] =
		physicalAddress
		| (accessPermission << 10)
		| (domain << 5)
		| (1 << 4)
		| (cachable << 3)
		| (bufferable << 2)
		| 0x2;
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::mapLargePage
//
// Maps an <virtualAddress> space to a <physicalAddress> space.
//------------------------------------------------------------------------------------------------

void AddressTranslationTableBuilder::mapLargePage(
	UInt virtualAddress,
	UInt physicalAddress,
	UInt domain,
	AccessPermission accessPermission,
	Bool cachable,
	Bool bufferable)
{
	L2TableEntry *pL2Table = mapL2Table(virtualAddress, domain);
	L2TableEntry *pEntries = &pL2Table[(virtualAddress >> 12) & ((sectionSize - 1) >> 12)];
	L2TableEntry largePageDescriptor =
		physicalAddress
		| (accessPermission << 10)
		| (accessPermission << 8)
		| (accessPermission << 6)
		| (accessPermission << 4)
		| (cachable << 3)
		| (bufferable << 2)
		| 0x1;
	for(UInt i = 0; i < largePageSize / smallPageSize; ++i)
	{
		pEntries[i] = largePageDescriptor;
	}
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::mapSmallPage
//
// Maps an <virtualAddress> space to a <physicalAddress> space.
//------------------------------------------------------------------------------------------------

void AddressTranslationTableBuilder::mapSmallPage(
	UInt virtualAddress,
	UInt physicalAddress,
	UInt domain,
	AccessPermission accessPermission,
	Bool cachable,
	Bool bufferable)
{
	L2TableEntry *pL2Table = mapL2Table(virtualAddress, domain);
	pL2Table[(virtualAddress >> 12) & ((sectionSize - 1) >> 12)] =
		physicalAddress
		| (accessPermission << 10)
		| (accessPermission << 8)
		| (accessPermission << 6)
		| (accessPermission << 4)
		| (cachable << 3)
		| (bufferable << 2)
		| 0x2;
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::mapQuarterSmallPage
//
// Maps an <virtualAddress> space to a <physicalAddress> space.
//------------------------------------------------------------------------------------------------

void AddressTranslationTableBuilder::mapQuarterSmallPage(
	UInt virtualAddress,
	UInt physicalAddress,
	UInt domain,
	AccessPermission accessPermission,
	Bool cachable,
	Bool bufferable)
{
	const UInt permissionShift = 4 + ((physicalAddress >> 9) & 0x6);
	L2TableEntry *pL2Table = mapL2Table(virtualAddress, domain);
	L2TableEntry *pEntry = &pL2Table[(virtualAddress >> 12) & ((sectionSize - 1) >> 12)];
	*pEntry = (*pEntry & 0xFF0 & ~(0x3 << permissionShift))
		| physicalAddress
		| (accessPermission << permissionShift)
		| (cachable << 3)
		| (bufferable << 2)
		| 0x2;
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::mapL2Table
//
// Gets an existing or create a new level 2 translation table for the specified address.
//------------------------------------------------------------------------------------------------

AddressTranslationTableBuilder::L2TableEntry *AddressTranslationTableBuilder::mapL2Table(
	UInt virtualAddress, UInt domain)
{
	// check if the level 2 table doesn't already exist
	if((pL1Table[virtualAddress >> 20] & 0x3) != 0x1)
	{
		// allocate a new level 2 table
		pL1Table[virtualAddress >> 20] =
			(UInt)allocateTable(1024)
			| (domain << 5)
			| (1 << 4)
			| 0x1;
	}

	return (L2TableEntry *)(pL1Table[virtualAddress >> 20] & ~((1 << 10) - 1));
}

//------------------------------------------------------------------------------------------------
// * AddressTranslationTableBuilder::allocateTable
//
// Allocates a new translation table and initializes all entries to contain fault descriptors.
//------------------------------------------------------------------------------------------------

AddressTranslationTableBuilder::TableEntry *AddressTranslationTableBuilder::allocateTable(
	UInt sizeInBytes)
{
	TableEntry *pTable;

	// check if the tables grow upward or downward in memory
	if(growsUpwards)
	{
		pTable = (TableEntry *)pTableLimit;
		pTableLimit = addToPointer(pTableLimit, sizeInBytes);
	}
	else
	{
		pTableBase = subtractFromPointer(pTableBase, sizeInBytes);
		pTable = (TableEntry *)pTableBase;
	}

	// zero initialize the table
	for(UInt i = 0; i < sizeInBytes / sizeof(TableEntry); ++i)
	{
		pTable[i] = null;
	}

	return pTable;
}
