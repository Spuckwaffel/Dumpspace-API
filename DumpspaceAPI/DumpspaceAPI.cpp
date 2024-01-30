#include <iostream>

#include "DSAPI.h"

int main()
{
    // first we create a DSAPI object with the hash of the game. You can get the hash out of the url
    // ...dumpspace/main/Games/index.html?hash=6b77eceb <----
    DSAPI api = DSAPI("6b77eceb");

    // downloads all JSONs to use the API
    api.downloadContent();

    // gets any offset of the OFFSETS tab
    const auto UWorldOffset = api.getOffset("OFFSET_UWORLD");
    printf("UWorld offset: 0x%llX\n", UWorldOffset);

    // gets the size of any defined class or struct
    const auto classSize = api.getSizeofClass("AActor");
    printf("AActor class size: 0x%X\n", classSize);

    // gets the OffsetInfo of a member within a class or struct
    const auto offset = api.getOffset("UWorld", "OwningGameInstance");

    // is it valid?
    if (!offset)
        DebugBreak();

    printf("OwningGameInstance offset: 0x%llX size: 0x%llX\n", offset.offset, offset.size);

    // get the function offset of a function
    const auto functionOffset = api.getFunctionOffset("AFortWeapon", "WeaponDataIsValid");
    printf("AFortWeapon::WeaponDataIsValid offset 0x%llX\n", functionOffset);

    // get the name of an enum from an enum class
    // imagine we read the rarity of a item and get the value 4

    //enum class EFortRarity : uint8_t {
    //    EFortRarity__Common = 0,
    //    EFortRarity__Uncommon = 1,
    //    EFortRarity__Rare = 2,
    //    EFortRarity__Epic = 3,
    //    EFortRarity__Legendary = 4, <---- it will return this name
    //    EFortRarity__Mythic = 5,
    //    EFortRarity__Transcendent = 6,
    //    ...
    //};
    const auto enumName = api.getEnumName("EFortRarity", 4);
    printf("EFortRarity type 4: %s\n", enumName.c_str());

    puts("end");
    return 0;
}