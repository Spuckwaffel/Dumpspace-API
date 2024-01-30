# DumpspaceAPI

The dumpspace API allows you to get your Games' info directly from the Dumpspace website to use it in your C++ project.

### Example
All examples are shown within the ``DumpspaceAPI.cpp`` file:

- First we create a DSAPI object with the hash of the game. You can get the hash out of the url

   - example url: ``...dumpspace/main/Games/index.html?hash=6b77eceb <----``
```c++
DSAPI api = DSAPI("6b77eceb");
```

- Downloads all JSONs to use the API
```c++
api.downloadContent();
```

- Gets the size of any defined class or struct
```c++
const auto classSize = api.getSizeofClass("AActor");
printf("AActor class size: 0x%X\n", classSize);
```

- Gets the OffsetInfo of a member within a class or struct
```c++
const auto offset = api.getOffset("UWorld", "OwningGameInstance");

// is it valid?
if (!offset)
    DebugBreak();

printf("OwningGameInstance offset: 0x%llX size: 0x%llX\n", offset.offset, offset.size);
```

- Get the function offset of a function
```c++
const auto functionOffset = api.getFunctionOffset("AFortWeapon", "WeaponDataIsValid");
printf("AFortWeapon::WeaponDataIsValid offset 0x%llX\n", functionOffset);
```

- Get the name of an enum from an enum class.
   - Imagine we read the rarity of a item and get the value 4
```c++
enum class EFortRarity : uint8_t {
    EFortRarity__Common = 0,
    EFortRarity__Uncommon = 1,
    EFortRarity__Rare = 2,
    EFortRarity__Epic = 3,
    EFortRarity__Legendary = 4, // <---- it will return this name
    EFortRarity__Mythic = 5,
    EFortRarity__Transcendent = 6,
    ...
};
```

```c++
const auto enumName = api.getEnumName("EFortRarity", 4);
printf("EFortRarity type 4: %s\n", enumName.c_str());
```
