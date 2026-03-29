#include <Arduino.h>
#include <random>

// ─── PINS ────────────────────────────────────────────────────────────────────
#define JOY_X       34   // Joystick X axis (analog)
#define JOY_Y       35   // Joystick Y axis (analog)

#define BTN_ATK     25   // K -> Attack
#define BTN_INT     26   // J -> Interact
#define BTN_INV     27   // I -> Inventory

// In-fight buttons (reuse same 3 physical buttons, context-sensitive)
// BTN_ATK -> 'a' attack
// BTN_INT -> 'h' heal
// BTN_INV -> 'r' run

#define DEADZONE    400  // Joystick deadzone (0-4095 range, center ~2048)
#define DEBOUNCE_MS 200

// ─── DUNGEON ─────────────────────────────────────────────────────────────────
String Dungeon[5][5][9][9] = {
{
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "{W}", "[_]", "[_]", "[_]", "[_]"},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "{X}", "[_]", "[_]", "[_]", "|+|"},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "|+|", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "{@}", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "{#}", "[_]", "[_]", "[_]", "[_]"},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
},
{
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "{#}", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]"},
    {"  | ", "[_]", "[|]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[|]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]"},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[_]", "[|]", "[_]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[_]", "[|]", "[_]", "[|]", "[_]", " | "},
    {" [_]", "[_]", "[|]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]"},
    {"  | ", "[_]", "[|]", "[_]", "[|]", "[_]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[_]", "[|]", "[_]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "{#}", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
},
{
 {
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "{%}", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]"},
    {"  | ", "{#}", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[|]", "[|]", "[|]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[_]", "[_]", "[_]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]"},
    {"  | ", "[_]", "[|]", "[_]", "[_]", "[_]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[_]", "[_]", "[_]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]"},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "|+|", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[|]", "{X}", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
},
{
 {
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "{X}", "[|]", "[_]", "[|]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "{#}", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "{%}", "[_]", "{W}", "[_]", "{#}", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "|+|", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "{X}", "[_]", "[_]", "[_]", "|+|"},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[|]", "[|]", "[|]", "[|]", "[|]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[|]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]"},
    {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[|]", "[|]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[|]", "[|]", "[|]", "[|]", "[|]", "[_]", " | "},
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
 },
 {
    {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
    {" [_]", "[_]", "[_]", "[_]", "{X}", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", " | "},
    {"  | ", "[_]", "[_]", "[_]", "[_]", "[|]", "{#}", "[_]", " | "},
    {"  |-", "---", "---", "---", "|+|", "---", "---", "---", "-| "}
 },
},
{
  {
      {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[|]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[|]", "[|]", "[_]", "[_]", "[_]", "[_]", "[_]"},
      {"  | ", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[|]", "[_]", "[|]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "{#}", "[_]", "[|]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
  },
  {
      {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {" |+|", "[_]", "[_]", "[_]", "{X}", "[_]", "[_]", "[_]", "[_]"},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "{#}", " | "},
      {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
  },
  {
      {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {" [_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]"},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
  },
  {
      {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "},
      {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
      {" [_]", "[_]", "[_]", "[_]", "[_]", "[_]", "{W}", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[|]", "[_]", "[_]", "[_]", " | "},
      {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
  },
  {
      {"  |-", "---", "---", "---", "[_]", "---", "---", "---", "-| "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "{W}", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  | ", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", "[_]", " | "},
      {"  |-", "---", "---", "---", "---", "---", "---", "---", "-| "}
  }
}
};

// ─── WEAPONS ─────────────────────────────────────────────────────────────────
String weapons[5][3] = {
  {"Fist",        "", "1"},
  {"Simple Whip", "", "200"},
  {"Short Sword", "", "3"},
  {"Bloody Axe",  "", "5"},
  {"Master Sword","", "10"}
};

// ─── STRUCTS / CLASSES ───────────────────────────────────────────────────────
struct Position {
  int x, y, X, Y;
};

class Player {
  public:
    Position pos;
    int health = 10;
    int potions = 100;
    const int MAXHEALTH = 10;
    int weapon = 1;
};

class Game {
  public:
    String cursors[4] = {"^", "v", ">", "<"};
    String cursor = cursors[0];

    void printLogoAndControls() {
      Serial.println(F("============================================"));
      Serial.println(F("   THE LEGEND OF BREADTH OF THE C++"));
      Serial.println(F("============================================"));
      Serial.println(F("CONTROLS:"));
      Serial.println(F("  Joystick        -> Move (WASD)"));
      Serial.println(F("  BTN_ATK (GPIO25) -> Attack (K) / [a] in fight"));
      Serial.println(F("  BTN_INT (GPIO26) -> Interact (J) / [h] heal in fight"));
      Serial.println(F("  BTN_INV (GPIO27) -> Inventory (I) / [r] run in fight"));
      Serial.println(F("LEGEND:"));
      Serial.println(F("  {#}=Potion  {W}=Weapon  {%}=Mystery  {X}=Monster  {@}=Ganon"));
      Serial.println(F("============================================"));
    }

    void printRoom(String room[9][9], int x, int y, int X, int Y) {
      for (int i = 0; i < 9; i++) {
        String line = "";
        for (int j = 0; j < 9; j++) {
          if (j == x && i == y) {
            if (x == 0 && y == 4)
              line += " [" + cursor + "]";
            else
              line += "[" + cursor + "]";
          } else {
            line += room[i][j];
          }
        }
        // Dungeon minimap on rows 2-6
        if (i == 1) {
          line += "    Dungeon:";
        } else if (i >= 2 && i < 7) {
          line += "    ";
          for (int l = 0; l < 5; l++) {
            line += (l == Y && (i - 2) == X) ? "##" : "__";
          }
        }
        Serial.println(line);
      }
    }

    void setRoom(int X, int Y, String room[9][9]) {
      for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
          room[i][j] = Dungeon[X][Y][i][j];
    }

    bool interactWithChest(int X, int Y, String room[9][9], String chest) {
      for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
          if (room[i][j] == chest) {
            Dungeon[X][Y][i][j] = "[_]";
            return true;
          }
      return false;
    }

    void removeChest(String room[9][9], int X, int Y, String chest) {
      for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
          if (room[i][j] == chest)
            Dungeon[X][Y][i][j] = "[_]";
    }
};

class Monster {
  public:
    int health = 10;
    int X, Y;
    String attacks[5] = {"slapped", "kicked", "sliced", "slammed", "spat at"};
    String limbs[5]   = {"face", "butt", "stomach", "nose", "shoulder"};

    Monster(int x, int y) : X(x), Y(y) {}

    String checkForMonster(int x, int y, String room[9][9], String cursor, String cursors[4]) {
      if (cursor == cursors[0] && room[y-1][x] == "{X}") return "{X}";
      if (cursor == cursors[1] && room[y+1][x] == "{X}") return "{X}";
      if (cursor == cursors[2] && room[y][x+1] == "{X}") return "{X}";
      if (cursor == cursors[3] && room[y][x-1] == "{X}") return "{X}";
      return "NA";
    }

    String checkForBoss(int x, int y, String room[9][9], String cursor, String cursors[4]) {
      if (cursor == cursors[0] && room[y-1][x] == "{@}") return "{@}";
      if (cursor == cursors[1] && room[y+1][x] == "{@}") return "{@}";
      if (cursor == cursors[2] && room[y][x+1] == "{@}") return "{@}";
      if (cursor == cursors[3] && room[y][x-1] == "{@}") return "{@}";
      return "NA";
    }

    int generateDamage() { return (random(0, 5) > 3) ? 2 : 1; }
    String generateAttack() { return attacks[random(0, 5)]; }
    String generateLimb()   { return limbs[random(0, 5)]; }

    void openDoorAndRemove(String room[9][9]) {
      for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++) {
          if (room[i][j] == "|+|")  Dungeon[X][Y][i][j] = "[_]";
          if (room[i][j] == " |+|") Dungeon[X][Y][i][j] = " [_]";
          if (room[i][j] == "{X}" || room[i][j] == "{@}") Dungeon[X][Y][i][j] = "[_]";
        }
    }
};

// ─── BUTTON HELPERS ──────────────────────────────────────────────────────────
unsigned long lastPress[3] = {0, 0, 0};

// Returns true once per physical press (debounced)
bool btnPressed(int pin, int idx) {
  if (digitalRead(pin) == LOW && millis() - lastPress[idx] > DEBOUNCE_MS) {
    lastPress[idx] = millis();
    return true;
  }
  return false;
}

// Read joystick and return 'w'/'a'/'s'/'d' or '\0'
char readJoystick() {
  int xVal = analogRead(JOY_X); // 0-4095
  int yVal = analogRead(JOY_Y);
  int center = 2048;

  if (yVal < center - DEADZONE) return 'w'; // up
  if (yVal > center + DEADZONE) return 's'; // down
  if (xVal < center - DEADZONE) return 'a'; // left
  if (xVal > center + DEADZONE) return 'd'; // right
  return '\0';
}

// ─── GLOBALS ─────────────────────────────────────────────────────────────────
Player player;
Game   game;
Monster monster(0, 0);
String currentRoom[9][9];
bool won = false;

unsigned long lastJoyMove = 0;
#define JOY_REPEAT_MS 200  // movement repeat rate while held

// ─── SETUP ───────────────────────────────────────────────────────────────────
void setup() {
  Serial.begin(921600);

  pinMode(BTN_ATK, INPUT_PULLUP);
  pinMode(BTN_INT, INPUT_PULLUP);
  pinMode(BTN_INV, INPUT_PULLUP);

  randomSeed(analogRead(0));

  game.printLogoAndControls();
  player.pos = {4, 7, 4, 2};  // x, y, X, Y
  game.setRoom(player.pos.X, player.pos.Y, currentRoom);

  Serial.println();
  game.printRoom(currentRoom, player.pos.x, player.pos.y, player.pos.X, player.pos.Y);
  Serial.println(F("====================================="));
}

// ─── FIGHT LOOP ──────────────────────────────────────────────────────────────
void fightLoop(bool isBoss) {
  monster = Monster(player.pos.X, player.pos.Y);
  monster.health = isBoss ? 150 : 10;

  if (isBoss) {
    Serial.println(F("Get ready to fight... GANON!!!"));
    Serial.println(F("====================================="));
  }

  bool inFight = true;
  while (inFight) {
    Serial.println(F("You initiated a fight!"));
    Serial.print(F("Your HP: ")); Serial.println(player.health);
    Serial.print(F("Monster HP: ")); Serial.println(monster.health);
    Serial.println(F("[ATK btn]=Attack  [INT btn]=Heal  [INV btn]=Run"));

    while (inFight) {
      int monsterDmg = monster.generateDamage();
      String monsterAtk  = monster.generateAttack();
      String monsterLimb = monster.generateLimb();

      // ── Attack ──
      if (btnPressed(BTN_ATK, 0)) {
        Serial.println(F("====================================="));
        Serial.print(F("You attacked with your "));
        Serial.print(weapons[player.weapon][0]);
        Serial.print(F("! Dealt "));
        Serial.print(weapons[player.weapon][2]);
        Serial.println(F(" damage!"));

        monster.health -= weapons[player.weapon][2].toInt();
        if (monster.health <= 0) {
          Serial.println(F("You defeated the monster!"));
          monster.openDoorAndRemove(currentRoom);
          inFight = false;
          break;
        }
        Serial.print(F("Monster "));
        Serial.print(monsterAtk);
        Serial.print(F(" you in the "));
        Serial.print(monsterLimb);
        Serial.print(F("! Dealt "));
        Serial.print(monsterDmg);
        Serial.println(F(" damage!"));
        player.health -= monsterDmg;
        if (player.health <= 0) { Serial.println(F("The monster got the best of you...")); while(true); }
        Serial.print(F("Your HP: ")); Serial.println(player.health);
        Serial.print(F("Monster HP: ")); Serial.println(monster.health);
      }

      // ── Heal ──
      else if (btnPressed(BTN_INT, 1)) {
        Serial.println(F("====================================="));
        if (player.potions > 0) {
          if (player.health >= player.MAXHEALTH - 1) {
            Serial.println(F("Healed 1hp!"));
            player.health = min(player.health + 1, player.MAXHEALTH);
          } else {
            Serial.println(F("Healed 2hp!"));
            player.health += 2;
          }
          player.potions--;
          Serial.print(F("Potions left: ")); Serial.println(player.potions);
          Serial.print(F("Monster ")); Serial.print(monsterAtk);
          Serial.print(F(" you in the ")); Serial.print(monsterLimb);
          Serial.print(F("! Dealt ")); Serial.print(monsterDmg); Serial.println(F(" damage!"));
          player.health -= monsterDmg;
          if (player.health <= 0) { Serial.println(F("The monster got the best of you...")); while(true); }
          Serial.print(F("Your HP: ")); Serial.println(player.health);
          Serial.print(F("Monster HP: ")); Serial.println(monster.health);
        } else {
          Serial.println(F("No potions left!"));
        }
      }

      // ── Run ──
      else if (btnPressed(BTN_INV, 2)) {
        Serial.println(F("====================================="));
        Serial.println(F("You ran away!"));
        // Flip direction
        if      (game.cursor == game.cursors[0]) game.cursor = game.cursors[1];
        else if (game.cursor == game.cursors[1]) game.cursor = game.cursors[0];
        else if (game.cursor == game.cursors[2]) game.cursor = game.cursors[3];
        else                                     game.cursor = game.cursors[2];
        inFight = false;
      }
    }
  }
}

// ─── MAIN LOOP ───────────────────────────────────────────────────────────────
void loop() {
  bool acted = false;
  char ch = '\0';

  // Joystick movement with repeat throttle
  if (millis() - lastJoyMove > JOY_REPEAT_MS) {
    ch = readJoystick();
    if (ch != '\0') lastJoyMove = millis();
  }

  // ── ATK button -> fight ──
  if (btnPressed(BTN_ATK, 0)) {
    bool nearMonster = monster.checkForMonster(player.pos.x, player.pos.y, currentRoom, game.cursor, game.cursors) == "{X}";
    bool nearBoss    = monster.checkForBoss(player.pos.x, player.pos.y, currentRoom, game.cursor, game.cursors) == "{@}";
    if (nearMonster || nearBoss) {
      fightLoop(nearBoss);
      acted = true;
    }
  }

  // ── INT button -> interact ──
  else if (btnPressed(BTN_INT, 1)) {
    if (game.interactWithChest(player.pos.X, player.pos.Y, currentRoom, "{#}")) {
      player.potions += 3;
      Serial.print(F("Found 3 potions! Total: ")); Serial.println(player.potions);
      game.removeChest(currentRoom, player.pos.X, player.pos.Y, "{#}");
    } else if (game.interactWithChest(player.pos.X, player.pos.Y, currentRoom, "{%}")) {
      if (random(0, 2)) {
        Serial.println(F("Lucky! Got 2 potions!"));
        player.potions += 2;
      } else {
        Serial.println(F("Unlucky! Goblin stole 2 potions!"));
        player.potions -= 2;
      }
      Serial.print(F("Potions: ")); Serial.println(player.potions);
      game.removeChest(currentRoom, player.pos.X, player.pos.Y, "{%}");
    } else if (game.interactWithChest(player.pos.X, player.pos.Y, currentRoom, "{W}")) {
      player.weapon++;
      Serial.print(F("New weapon: ")); Serial.println(weapons[player.weapon][0]);
      game.removeChest(currentRoom, player.pos.X, player.pos.Y, "{W}");
    }
    Serial.println(F("====================================="));
    acted = true;
  }

  // ── INV button -> inventory ──
  else if (btnPressed(BTN_INV, 2)) {
    Serial.println(F("\n--- Inventory ---"));
    Serial.print(F("Weapon: ")); Serial.print(weapons[player.weapon][0]);
    Serial.print(F(" | DMG: ")); Serial.println(weapons[player.weapon][2]);
    Serial.print(F("Potions: ")); Serial.println(player.potions);
    Serial.println(F("-----------------\n"));
    Serial.println(F("====================================="));
    acted = true;
  }

  // ── Joystick movement ──
  else if (ch != '\0') {
    switch (ch) {
      case 'w':
        game.cursor = game.cursors[0];
        if (player.pos.y == 0 && player.pos.X > 0) { player.pos.X--; player.pos.y = 8; }
        else if (player.pos.y > 0 && currentRoom[player.pos.y-1][player.pos.x] == "[_]") player.pos.y--;
        else if (player.pos.X == 0 && player.pos.Y == 2 && player.pos.x == 4 && player.pos.y == 0) won = true;
        break;
      case 'a':
        game.cursor = game.cursors[3];
        if (player.pos.x == 0 && player.pos.Y > 0) { player.pos.Y--; player.pos.x = 8; }
        else if (player.pos.x > 0 && (currentRoom[player.pos.y][player.pos.x-1] == " [_]" || currentRoom[player.pos.y][player.pos.x-1] == "[_]")) player.pos.x--;
        break;
      case 's':
        game.cursor = game.cursors[1];
        if (player.pos.y == 8 && player.pos.X < 4) { player.pos.X++; player.pos.y = 0; }
        else if (player.pos.y < 8 && currentRoom[player.pos.y+1][player.pos.x] == "[_]") player.pos.y++;
        break;
      case 'd':
        game.cursor = game.cursors[2];
        if (player.pos.x == 8 && player.pos.Y < 4) { player.pos.Y++; player.pos.x = 0; }
        else if (player.pos.x < 8 && currentRoom[player.pos.y][player.pos.x+1] == "[_]") player.pos.x++;
        break;
    }
    acted = true;
  }

  if (won) {
    Serial.println(F("====================================="));
    Serial.println(F("  FREEDOM AT LAST! YOU ESCAPED!"));
    Serial.println(F("  THANKS FOR PLAYING"));
    Serial.println(F("====================================="));
    while (true);
  }

  if (acted) {
    game.setRoom(player.pos.X, player.pos.Y, currentRoom);
    game.printRoom(currentRoom, player.pos.x, player.pos.y, player.pos.X, player.pos.Y);
    Serial.println(F("====================================="));
  }
}
