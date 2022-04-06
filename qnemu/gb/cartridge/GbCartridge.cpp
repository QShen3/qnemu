/* 
 *  Copyright [2022] <qazxdrcssc2006@163.com>
 */

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "qnemu/gb/cartridge/GbCartridge.h"
#include "qnemu/gb/cartridge/mbc/GbMbcFactoryInterface.h"
#include "qnemu/gb/cartridge/mbc/GbMbcInterface.h"

namespace qnemu
{

constexpr uint8_t nintendoLogoData[] = {
    0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
    0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
    0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
};

const std::map<uint8_t, std::string> licenseCodeMap {
    {0x00, "None"},                  {0x01, "Nintendo R&D1"},         {0x08, "Capcom"},                   {0x09, "hot-b"},
    {0x0A, "jaleco"},                {0x0B, "coconuts"},              {0x0C, "elite systems"},            {0x13, "Electronic Arts"},
    {0x18, "Hudson Soft"},           {0x19, "itc entertainment"},     {0x1A, "yanoman"},                  {0x1D, "clary"},
    {0x1F, "virgin"},                {0x24, "PCM Complete"},          {0x25, "san-x"},                    {0x28, "kotobuki systems"},
    {0x29, "seta"},                  {0x30, "infogrames"},            {0x31, "Nintendo"},                 {0x32, "Bandai"},
    {0x34, "Konami"},                {0x35, "Hector"},                {0x38, "capcom"},                   {0x39, "Banpresto"},
    {0x3C, "*entertainment i"},      {0x3E, "gremlin"},               {0x41, "Ubi Soft"},                 {0x42, "Atlus"},
    {0x44, "Malibu"},                {0x46, "angel"},                 {0x47, "spectrum holoby"},          {0x49, "irem"},
    {0x4A, "virgin"},                {0x4D, "malibu"},                {0x4F, "u.s. gold"},                {0x50, "Absolute"},
    {0x51, "Acclaim"},               {0x52, "Activision"},            {0x53, "American sammy"},           {0x54, "gametek"},
    {0x55, "park place"},            {0x56, "LJN"},                   {0x57, "Matchbox"},                 {0x59, "Milton Bradley"},
    {0x5A, "mindscape"},             {0x5B, "romstar"},               {0x5C, "naxat soft"},               {0x5D, "tradewest"},
    {0x60, "Titus"},                 {0x61, "Virgin"},                {0x67, "Ocean"},                    {0x69, "Electronic Arts"},
    {0x6E, "elite systems"},         {0x6F, "electro brain"},         {0x70, "Infogrames"},               {0x71, "Interplay"},
    {0x72, "Broderbund"},            {0x73, "sculptured"},            {0x75, "the sales curve"},          {0x78, "THQ"},
    {0x79, "Accolade"},              {0x7A, "triffix entertainment"}, {0x7C, "microprose"},               {0x7F, "kemco"},
    {0x80, "misawa"},                {0x83, "lozc"},                  {0x86, "Tokuma Shoten Intermedia"}, {0x8B, "bullet-proof software"},
    {0x8C, "vic tokai"},             {0x8E, "ape"},                   {0x8F, "i'max"},                    {0x91, "Chunsoft"},
    {0x92, "Video system"},          {0x93, "tsuburava"},             {0x95, "Varie"},                    {0x96, "Yonezawa/s'pal"},
    {0x97, "Kaneko"},                {0x99, "arc"},                   {0x9A, "nihon bussan"},             {0x9B, "tecmo"},
    {0x9C, "imagineer"},             {0x9D, "banpresto"},             {0x9F, "nova"},                     {0xA1, "hori electric"},
    {0xA2, "bandai"},                {0xA4, "Konami (Yu-Gi-Oh!)"},    {0xA6, "kawada"},                   {0xA7, "takara"},
    {0xA9, "technos japan"},         {0xAA, "broderbund"},            {0xAC, "toei animation"},           {0xAD, "toho"},
    {0xAF, "namco"},                 {0xB0, "acclaim"},               {0xB1, "ascii or nexoft"},          {0xB2, "bandai"},
    {0xB4, "enix"},                  {0xB6, "hal"},                   {0xB7, "snk"},                      {0xB9, "pony canyon"},
    {0xBA, "*culture brain o"},      {0xBB, "sunsoft"},               {0xBD, "sony imagesoft"},           {0xBF, "sammy"},
    {0xC0, "taito"},                 {0xC2, "kemco"},                 {0xC3, "squaresoft"},               {0xC4, "*tokuma shoten i"},
    {0xC5, "data east"},             {0xC6, "tonkin house"},          {0xC8, "koei"},                     {0xC9, "ufl"},
    {0xCA, "ultra"},                 {0xCB, "vap"},                   {0xCC, "use"},                      {0xCD, "meldac"},
    {0xCE, "*pony canyon or"},       {0xCF, "angel"},                 {0xD0, "taito"},                    {0xD1, "sofel"},
    {0xD2, "quest"},                 {0xD3, "sigma enterprises"},     {0xD4, "ask kodansha"},             {0xD6, "naxat soft"},
    {0xD7, "copya systems"},         {0xD9, "banpresto"},             {0xDA, "tomy"},                     {0xDB, "ljn"},
    {0xDD, "ncs"},                   {0xDE, "human"},                 {0xDF, "altron"},                   {0xE0, "jaleco"},
    {0xE1, "towachiki"},             {0xE2, "uutaka"},                {0xE3, "varie"},                    {0xE5, "epoch"},
    {0xE7, "athena"},                {0xE8, "asmik"},                 {0xE9, "natsume"},                  {0xEA, "king records"},
    {0xEB, "atlus"},                 {0xEC, "epic/sony records"},     {0xEE, "igs"},                      {0xF0, "a wave"},
    {0xF3, "extreme entertainment"}, {0xFF, "ljn"},
};

const std::map<std::string, std::string> newLicenseCodeMap {
    {"00", "None"},                  {"01", "Nintendo R&D1"},   {"08", "Capcom"},         {"13", "Electronic Arts"},
    {"18", "Hudson Soft"},           {"19", "b-ai"},            {"20", "kss"},            {"22", "pow"},
    {"24", "PCM Complete"},          {"25", "san-x"},           {"28", "Kemco Japan"},    {"29", "seta"},
    {"30", "Viacom"},                {"31", "Nintendo"},        {"32", "Bandai"},         {"33", "Ocean/Acclaim"},
    {"34", "Konami"},                {"35", "Hector"},          {"37", "Taito"},          {"38", "Hudson"},
    {"39", "Banpresto"},             {"41", "Ubi Soft"},        {"42", "Atlus"},          {"44", "Malibu"},
    {"46", "angel"},                 {"47", "Bullet-Proof"},    {"49", "irem"},           {"50", "Absolute"},
    {"51", "Acclaim"},               {"52", "Activision"},      {"53", "American sammy"}, {"54", "Konami"},
    {"55", "Hi tech entertainment"}, {"56", "LJN"},             {"57", "Matchbox"},       {"58", "Mattel"},
    {"59", "Milton Bradley"},        {"60", "Titus"},           {"61", "Virgin"},         {"64", "LucasArts"},
    {"67", "Ocean"},                 {"69", "Electronic Arts"}, {"70", "Infogrames"},     {"71", "Interplay"},
    {"72", "Broderbund"},            {"73", "sculptured"},      {"75", "sci"},            {"78", "THQ"},
    {"79", "Accolade"},              {"80", "misawa"},          {"83", "lozc"},           {"86", "Tokuma Shoten Intermedia"},
    {"87", "Tsukuda Original"},      {"91", "Chunsoft"},        {"92", "Video system"},   {"93", "Ocean/Acclaim"},
    {"95", "Varie"},                 {"96", "Yonezawa/s'pal"},  {"97", "Kaneko"},         {"99", "Pack in soft"},
    {"A4", "Konami (Yu-Gi-Oh!)"},
};

GbCartridge::GbCartridge(const GbMbcFactoryInterface& gbMbcFactory) :
    gbMbcFactory(gbMbcFactory),
    gbcCartridge(false),
    loaded(false)
{
}

GbCartridge::~GbCartridge()
{
    if (file.is_open()) {
        file.close();
    }
}

bool GbCartridge::accepts(uint16_t address) const
{
    return mbc->accepts(address);
}

uint8_t GbCartridge::read(uint16_t address) const
{
    return mbc->read(address);
}

void GbCartridge::write(uint16_t address, const uint8_t& value)
{
    mbc->write(address, value);
}

void GbCartridge::step()
{
    mbc->step();
}

void GbCartridge::reset()
{
    if (mbc) {
        mbc->reset();
    }
    gbcCartridge = false;
    loaded = false;
}

void GbCartridge::load(const char* filePath)
{
    if (file.is_open()) {
        file.close();
    }
    file.open(filePath, std::ios::in | std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Load cartridge file failed!");
    }
    romSize = file.tellg();
    size_t romBankCounts = romSize / GbMbcInterface::romBankSize;
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(romSize);
    file.read(reinterpret_cast<char *>(buffer.data()), static_cast<std::streamsize>(romSize));

    if (buffer.at(0x143) == 0xC0 || buffer.at(0x143) == 0x80) {
        gbcCartridge = true;
    } else {
        gbcCartridge = false;
    }

    if (!validate(buffer)) {
        throw std::runtime_error("Cartridge file is not correct!");
    }

    char gameTitle[15];
    for (size_t i = 0x134; i <= 0x142; i++) {
        gameTitle[i - 0x134] = static_cast<char>(buffer.at(i));
    }
    title = std::string(gameTitle);
    publisher = getPublisherFromCartridge(buffer);

    ramSize = getRamSizeFromCartridge(buffer.at(0x149));
    size_t ramBankCounts = ramSize == 0x800 ? 1 : ramSize / GbMbcInterface::ramBankSize;

    type = buffer.at(0x147);
    japanCartridge = buffer.at(0x14A) == 0 ? true : false;

    std::vector<std::array<uint8_t, GbMbcInterface::romBankSize>> romBanks(romBankCounts);
    std::vector<std::array<uint8_t, GbMbcInterface::ramBankSize>> ramBanks(ramBankCounts);
    file.seekg(0, std::ios::beg);
    for (auto& bank : romBanks) {
        std::array<uint8_t, GbMbcInterface::romBankSize> data;
        file.read(reinterpret_cast<char*>(data.data()), GbMbcInterface::romBankSize);
        std::copy(data.begin(), data.end(), bank.begin());
    }
    mbc = gbMbcFactory.create(std::move(romBanks), std::move(ramBanks), type);

    loaded = true;
}

bool GbCartridge::isGbcCartridge() const
{
    return gbcCartridge;
}

std::string GbCartridge::getPublisherFromCartridge(const std::vector<uint8_t>& buffer) const
{
    std::string publisher;
    if (buffer.at(0x14B) == 0x33) {
        char newLicenseCode[3] = {static_cast<char>(buffer.at(0x144)), static_cast<char>(buffer.at(0x145)), 0};
#if defined(__GNUC__) && !defined(__llvm__) && __GNUC__ < 9
        if (newLicenseCodeMap.find(std::string(newLicenseCode)) != newLicenseCodeMap.end()) {
#else
        if (newLicenseCodeMap.contains(std::string(newLicenseCode))) {
#endif
            publisher = newLicenseCodeMap.at(std::string(newLicenseCode));
        }
        else {
            publisher = "Unknown";
        }
    }
    else {
        uint8_t licenseCode = buffer.at(0x14B);
#if defined(__GNUC__) && !defined(__llvm__) && __GNUC__ < 9
        if (licenseCodeMap.find(licenseCode) != licenseCodeMap.end()) {
#else
        if (licenseCodeMap.contains(licenseCode)) {
#endif
            publisher = licenseCodeMap.at(licenseCode);
        }
        else {
            publisher = "Unknown";
        }
    }
    return publisher;
}

size_t GbCartridge::getRamSizeFromCartridge(uint8_t flag) const
{
    if (flag == 0) {
        return 0;
    }
    if (flag >= 1 && flag <= 4) {
        return 0x200 << (flag * 2);
    }
    if (flag == 5) {
        return 0x1'0000;
    }
    throw std::runtime_error("Cartridge file is not correct!");
}

size_t GbCartridge::getRomSizeFromCartridge(uint8_t flag) const
{
    if (flag <= 8) {
        return (GbMbcInterface::romBankSize * 2) << flag;
    }
    if (flag >= 0x52 && flag <= 0x54) {
        return 0x10'0000 + (0x2'0000 << (flag - 0x52));
    }
    throw std::runtime_error("Cartridge file is not correct!");
}

bool GbCartridge::validate(const std::vector<uint8_t>& buffer) const
{
    for (size_t i = 0x104; i < 0x134; i++) {
        if (buffer.at(i) != nintendoLogoData[i - 0x104]) {
            return false;
        }
    }

    if (getRomSizeFromCartridge(buffer.at(0x148)) != romSize) {
        return false;
    }

    uint8_t headerCheckSum = 0;
    for (size_t i = 0x134; i <= 0x014C; i++) {
        headerCheckSum = headerCheckSum - buffer.at(i) - 1;
    }
    if (buffer.at(0x14D) != headerCheckSum) {
        return false;
    }

    if (gbcCartridge) {
        uint16_t checkSum = 0;
        for (auto value : buffer) {
            checkSum = checkSum + value;
        }
        checkSum = checkSum - buffer.at(0x14E) - buffer.at(0x14F);
        if (checkSum != ((buffer.at(0x14E) << 8) | buffer.at(0x14F))) {
            return false;
        }
    }

    return true;
}

}  // namespace qnemu