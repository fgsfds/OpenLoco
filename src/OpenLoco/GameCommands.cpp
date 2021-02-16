#include "GameCommands.h"
#include "Audio/Audio.h"
#include "Company.h"
#include "CompanyManager.h"
#include "Map/Tile.h"
#include "Objects/ObjectManager.h"
#include "Objects/RoadObject.h"
#include "Objects/TrackObject.h"
#include "StationManager.h"
#include "Ui/WindowManager.h"
#include "Vehicles/Vehicle.h"
#include <cassert>

using namespace OpenLoco::Ui;
using namespace OpenLoco::Map;

namespace OpenLoco::GameCommands
{
    static loco_global<company_id_t, 0x009C68EB> _updating_company_id;
    static loco_global<uint8_t, 0x00508F08> game_command_nest_level;
    static loco_global<company_id_t[2], 0x00525E3C> _player_company;
    static loco_global<uint8_t, 0x00508F17> paused_state;
    static loco_global<uint8_t, 0x00508F1A> game_speed;
    static loco_global<uint16_t, 0x0050A004> _50A004;

    static uint16_t _gameCommandFlags;
    static loco_global<uintptr_t[80], 0x004F9548> _4F9548;
    static loco_global<uint8_t[80], 0x004F9688> _4F9688;

    static loco_global<tile_element*, 0x009C68D0> _9C68D0;

    static loco_global<coord_t, 0x009C68E4> _game_command_map_z;
    static loco_global<string_id, 0x009C68E6> gGameCommandErrorText;
    static loco_global<string_id, 0x009C68E8> gGameCommandErrorTitle;
    static loco_global<uint8_t, 0x009C68EE> _errorCompanyId;
    static loco_global<string_id[8], 0x112C826> _commonFormatArgs;

    using GameCommandFunc = void (*)(registers& regs);
    static GameCommandFunc _gameCommandFunctions[80] = {
        nullptr,          // vehicle_rearrange = 0,
        nullptr,          // vehicle_place = 1,
        nullptr,          // vehicle_pickup = 2,
        nullptr,          // vehicle_reverse = 3,
        nullptr,          // 4
        Vehicles::create, // vehicle_create = 5,
        nullptr,          // vehicle_sell = 6,
        nullptr,          // 7
        nullptr,          // 8
        nullptr,          // build_vehicle = 9,
        nullptr,          // vehicle_rename = 10,
        nullptr,          // change_station_name = 11,
        nullptr,          // vehicle_local_express = 12,
        nullptr,          // 13
        nullptr,          // 14
        nullptr,          // 15
        nullptr,          // 16
        nullptr,          // 17
        nullptr,          // 18
        nullptr,          // change_company_colour_scheme = 19,
        nullptr,          // pause_game = 20,
        nullptr,          // load_save_quit_game = 21,
        nullptr,          // 22
        nullptr,          // 23
        nullptr,          // change_land_material = 24,
        nullptr,          // raise_land = 25,
        nullptr,          // lower_land = 26,
        nullptr,          // lower_raise_land_mountain = 27,
        nullptr,          // raise_water = 28,
        nullptr,          // lower_water = 29,
        nullptr,          // 30
        nullptr,          // 31
        nullptr,          // 32
        nullptr,          // 33
        nullptr,          // 34
        nullptr,          // vehicle_order_insert = 35,
        nullptr,          // vehicle_order_delete = 36,
        nullptr,          // vehicle_order_skip = 37,
        nullptr,          // 38
        nullptr,          // 39
        nullptr,          // 40
        nullptr,          // 41
        nullptr,          // 42
        nullptr,          // 43
        nullptr,          // 44
        nullptr,          // 45
        nullptr,          // change_company_name = 46,
        nullptr,          // 47
        nullptr,          // 48
        nullptr,          // 49
        nullptr,          // 50
        nullptr,          // 51
        nullptr,          // 52
        nullptr,          // 53
        nullptr,          // remove_industry = 54,
        nullptr,          // build_company_headquarters = 55,
        nullptr,          // 56
        nullptr,          // 57
        nullptr,          // 58
        nullptr,          // vehicle_abort_pickup_air = 59,
        nullptr,          // 60
        nullptr,          // 61
        nullptr,          // 62
        nullptr,          // vehicle_abort_pickup_water = 63,
        nullptr,          // 63
        nullptr,          // 64
        nullptr,          // change_company_face = 66,
        nullptr,          // load_multiplayer_map = 67,
        nullptr,          // 68
        nullptr,          // 69
        nullptr,          // 70
        nullptr,          // send_chat_message = 71,
        nullptr,          // multiplayer_save = 72,
        nullptr,          // update_owner_status = 73,
        nullptr,          // vehicle_speed_control = 74,
        nullptr,          // vehicle_order_up = 75,
        nullptr,          // vehicle_order_down = 76,
        nullptr,          // vehicle_apply_shunt_cheat = 77,
        nullptr,          // apply_free_cash_cheat = 78,
        nullptr,          // rename_industry = 79,
    };

    void registerHooks()
    {
        registerHook(
            0x00431315,
            [](registers& regs) FORCE_ALIGN_ARG_POINTER -> uint8_t {
                registers backup = regs;
                auto ebx = doCommand(regs.esi, backup);

                regs = backup;
                regs.ebx = ebx;
                return 0;
            });
    }

    static uint32_t loc_4314EA();
    static uint32_t loc_4313C6(int esi, const registers& regs);

    // 0x00431315
    uint32_t doCommand(int esi, const registers& regs)
    {
        uint16_t flags = regs.bx;

        _gameCommandFlags = regs.bx;
        if (game_command_nest_level != 0)
            return loc_4313C6(esi, regs);

        if ((flags & GameCommandFlag::apply) == 0)
        {
            return loc_4313C6(esi, regs);
        }

        if ((flags & (GameCommandFlag::flag_4 | GameCommandFlag::flag_6)) != 0
            && _4F9688[esi] == 1
            && _updating_company_id == _player_company[0])
        {
            if (getPauseFlags() & 1)
            {
                paused_state = paused_state ^ 1;
                WindowManager::invalidate(WindowType::timeToolbar);
                Audio::unpauseSound();
                _50A004 = _50A004 | 1;
            }

            if (game_speed != 0)
            {
                game_speed = 0;
                WindowManager::invalidate(WindowType::timeToolbar);
            }

            if (isPaused())
            {
                gGameCommandErrorText = StringIds::empty;
                return 0x80000000;
            }
        }

        if (_updating_company_id == _player_company[0] && isNetworked())
        {
            assert(false);
            registers fnRegs = regs;
            call(0x0046E34A, fnRegs); // some network stuff. Untested
        }

        return loc_4313C6(esi, regs);
    }

    void callGameCommandFunction(uint32_t command, registers& regs)
    {
        if (_gameCommandFunctions[command] != nullptr)
        {
            _gameCommandFunctions[command](regs);
        }
        else
        {
            auto addr = _4F9548[command];
            call(addr, regs);
        }
    }

    static uint32_t loc_4313C6(int esi, const registers& regs)
    {
        uint16_t flags = regs.bx;
        gGameCommandErrorText = StringIds::null;
        game_command_nest_level++;

        uint16_t flagsBackup = _gameCommandFlags;
        registers fnRegs1 = regs;
        fnRegs1.bl &= ~GameCommandFlag::apply;
        callGameCommandFunction(esi, fnRegs1);
        int32_t ebx = fnRegs1.ebx;
        _gameCommandFlags = flagsBackup;

        if (ebx != static_cast<int32_t>(0x80000000))
        {
            if (isEditorMode())
                ebx = 0;

            if (game_command_nest_level == 1)
            {
                if ((_gameCommandFlags & GameCommandFlag::flag_2) == 0
                    && (_gameCommandFlags & GameCommandFlag::flag_6) == 0
                    && ebx != 0)
                {
                    registers regs2;
                    regs2.ebp = ebx;
                    call(0x0046DD06, regs2);
                    ebx = regs2.ebp;
                }
            }
        }

        if (ebx == static_cast<int32_t>(0x80000000))
        {
            if (flags & GameCommandFlag::apply)
            {
                return loc_4314EA();
            }
            else
            {
                game_command_nest_level--;
                return ebx;
            }
        }

        if ((flags & 1) == 0)
        {
            game_command_nest_level--;
            return ebx;
        }

        uint16_t flagsBackup2 = _gameCommandFlags;
        registers fnRegs2 = regs;
        callGameCommandFunction(esi, fnRegs2);
        int32_t ebx2 = fnRegs2.ebx;
        _gameCommandFlags = flagsBackup2;

        if (ebx2 == static_cast<int32_t>(0x80000000))
        {
            return loc_4314EA();
        }

        if (isEditorMode())
        {
            ebx = 0;
        }

        if (ebx2 < ebx)
        {
            ebx = ebx2;
        }

        game_command_nest_level--;
        if (game_command_nest_level != 0)
            return ebx;

        if ((flagsBackup2 & GameCommandFlag::flag_5) != 0)
            return ebx;

        {
            // Apply to company money
            registers fnRegs;
            fnRegs.ebx = ebx;
            call(0x0046DE2B, fnRegs);
        }

        if (ebx != 0 && _updating_company_id == _player_company[0])
        {
            // Add flying cost text
            registers fnRegs;
            fnRegs.ebx = ebx;
            _game_command_map_z = _game_command_map_z + 24;
            call(0x0046DC9F, fnRegs);
            _game_command_map_z = _game_command_map_z - 24;
        }

        return ebx;
    }

    static uint32_t loc_4314EA()
    {
        game_command_nest_level--;
        if (game_command_nest_level != 0)
            return 0x80000000;

        if (_updating_company_id != _player_company[0])
            return 0x80000000;

        if (_gameCommandFlags & GameCommandFlag::flag_3)
            return 0x80000000;

        if (gGameCommandErrorText != 0xFFFE)
        {
            Windows::showError(gGameCommandErrorTitle, gGameCommandErrorText);
            return 0x80000000;
        }

        // advanced errors
        if (_9C68D0 != (void*)-1)
        {
            auto tile = (tile_element*)_9C68D0;

            switch (tile->type())
            {
                case element_type::track: // 4
                {
                    auto trackElement = tile->asTrack();
                    if (trackElement == nullptr)
                        break; // throw exception?

                    track_object* pObject = ObjectManager::get<track_object>(trackElement->trackObjectId());
                    if (pObject == nullptr)
                        break;

                    _commonFormatArgs[0] = pObject->name;
                    _commonFormatArgs[1] = CompanyManager::get(_errorCompanyId)->name;
                    Windows::Error::openWithCompetitor(gGameCommandErrorTitle, StringIds::error_reason_stringid_belongs_to, _errorCompanyId);
                    return 0x80000000;
                }

                case element_type::road: //0x1C
                {
                    auto roadElement = tile->asRoad();
                    if (roadElement == nullptr)
                        break; // throw exception?

                    road_object* pObject = ObjectManager::get<road_object>(roadElement->roadObjectId());
                    if (pObject == nullptr)
                        break;

                    _commonFormatArgs[0] = pObject->name;
                    _commonFormatArgs[1] = CompanyManager::get(_errorCompanyId)->name;
                    Windows::Error::openWithCompetitor(gGameCommandErrorTitle, StringIds::error_reason_stringid_belongs_to, _errorCompanyId);
                    return 0x80000000;
                }

                case element_type::station: // 8
                {
                    auto stationElement = tile->asStation();
                    if (stationElement == nullptr)
                        break; // throw exception?

                    station* pStation = StationManager::get(stationElement->stationId());
                    if (pStation == nullptr)
                        break;

                    _commonFormatArgs[0] = pStation->name;
                    _commonFormatArgs[1] = pStation->town;
                    _commonFormatArgs[2] = CompanyManager::get(_errorCompanyId)->name;
                    Windows::Error::openWithCompetitor(gGameCommandErrorTitle, StringIds::error_reason_stringid_belongs_to, _errorCompanyId);
                    return 0x80000000;
                }

                case element_type::signal: // 0x0C
                {
                    _commonFormatArgs[0] = CompanyManager::get(_errorCompanyId)->name;
                    Windows::Error::openWithCompetitor(gGameCommandErrorTitle, StringIds::error_reason_signal_belongs_to, _errorCompanyId);
                    return 0x80000000;
                }

                default:
                    break;
            }
        }

        // fallback
        _commonFormatArgs[0] = CompanyManager::get(_errorCompanyId)->name;
        Windows::Error::openWithCompetitor(gGameCommandErrorTitle, StringIds::error_reason_stringid_belongs_to, _errorCompanyId);
        return 0x80000000;
    }

    // 0x00431E6A
    // al  : company
    // esi : tile
    bool sub_431E6A(const company_id_t company, Map::tile_element* const tile /*= nullptr*/)
    {
        if (company == CompanyId::neutral)
        {
            return true;
        }
        if (_updating_company_id == company || _updating_company_id == CompanyId::neutral)
        {
            return true;
        }
        gGameCommandErrorText = -2;
        _errorCompanyId = company;
        _9C68D0 = tile == nullptr ? reinterpret_cast<Map::tile_element*>(-1) : tile;
        return false;
    }
}
