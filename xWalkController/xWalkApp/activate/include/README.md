# xWalkApp Activation Includes

This directory owns declarations that activate validated Controller commands.
`xControllerCommands.h` declares the application-owned
free functions used to execute a command through a configured `XWalkController`
and obtain generated usage text.
`xControllerPicarxCommands.h` declares the separate application-owned router
that selects one protected PiCar-X command handler without exposing those
handlers as public class methods.

Boot declarations live under `../../boot/include`, parsing declarations under
`../../parse/include`, and Handler declarations under `../../../xWalkHandler/include`.
