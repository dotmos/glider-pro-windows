#
# Build script for Mermaid.dat assets zip file
#

set(RESOURCE_DIRECTORY "${CMAKE_SOURCE_DIR}/Resources")
set(MERMAID_DIRECTORY "${CMAKE_BINARY_DIR}/Mermaid-build")
set(MERMAID_OUTPUT "${CMAKE_BINARY_DIR}/Mermaid.dat")
set(MERMAID_FILELIST "${MERMAID_DIRECTORY}/filelist.txt")

function(copy_res_to_mermaid src_path dst_path)
  list(APPEND MERMAID_DEPENDENCIES "${RESOURCE_DIRECTORY}/${src_path}")
  set(MERMAID_DEPENDENCIES ${MERMAID_DEPENDENCIES} PARENT_SCOPE)
  configure_file(
    "${RESOURCE_DIRECTORY}/${src_path}"
    "${MERMAID_DIRECTORY}/${dst_path}"
    COPYONLY
  )
  file(APPEND "${MERMAID_FILELIST}" "${MERMAID_DIRECTORY}/${dst_path}\n")
endfunction()

set(IMAGES_DIRECTORY "${MERMAID_DIRECTORY}/images")
set(SOUNDS_DIRECTORY "${MERMAID_DIRECTORY}/sounds")
set(MERMAID_DEPENDENCIES "")

file(MAKE_DIRECTORY "${MERMAID_DIRECTORY}")
file(MAKE_DIRECTORY "${IMAGES_DIRECTORY}")
file(MAKE_DIRECTORY "${SOUNDS_DIRECTORY}")

file(WRITE "${MERMAID_FILELIST}" "")

#
# Image files
#

####copy_res_to_mermaid("Pictures/OkayButtNotHiLit.bmp" "images/150.bmp")
####copy_res_to_mermaid("Pictures/OkayButtHiLit.bmp" "images/151.bmp")
####copy_res_to_mermaid("Pictures/AboutBanner.bmp" "images/153.bmp")
copy_res_to_mermaid("Pictures/SplashScreen.bmp" "images/1000.bmp")
####copy_res_to_mermaid("Pictures/LoadTitle.bmp" "images/1001.bmp")
####copy_res_to_mermaid("Pictures/ObjectInfoTitle.bmp" "images/1002.bmp")
####copy_res_to_mermaid("Pictures/DefaultHouse1.bmp" "images/1003.bmp")
####copy_res_to_mermaid("Pictures/DefaultHouse8.bmp" "images/1004.bmp")
####copy_res_to_mermaid("Pictures/HouseInfoTitle.bmp" "images/1005.bmp")
####copy_res_to_mermaid("Pictures/DisplayPrefsTitle.bmp" "images/1006.bmp")
####copy_res_to_mermaid("Pictures/RoomInfoTitle.bmp" "images/1007.bmp")
####copy_res_to_mermaid("Pictures/SoundPrefsTitle.bmp" "images/1008.bmp")
####copy_res_to_mermaid("Pictures/RoomInfoInstruct.bmp" "images/1009.bmp")
copy_res_to_mermaid("Pictures/Thumbnail.bmp" "images/1010.bmp")
copy_res_to_mermaid("Pictures/Tools.bmp" "images/1011.bmp")
####copy_res_to_mermaid("Pictures/ControlPrefsTitle.bmp" "images/1012.bmp")
####copy_res_to_mermaid("Pictures/MainPrefsTitle.bmp" "images/1013.bmp")
####copy_res_to_mermaid("Pictures/BrainsPrefsTitle.bmp" "images/1014.bmp")
copy_res_to_mermaid("Pictures/EscPause.bmp" "images/1015.bmp")
copy_res_to_mermaid("Pictures/TabPause.bmp" "images/1016.bmp")
copy_res_to_mermaid("Pictures/StarsRemaining.bmp" "images/1017.bmp")
copy_res_to_mermaid("Pictures/StarRemaining.bmp" "images/1018.bmp")
copy_res_to_mermaid("Pictures/Angel.bmp" "images/1019.bmp")
copy_res_to_mermaid("Pictures/AngelMask.bmp" "images/1020.bmp")
copy_res_to_mermaid("Pictures/Milkyway.bmp" "images/1021.bmp")
####copy_res_to_mermaid("Pictures/MicrowaveInfoTitle.bmp" "images/1022.bmp")
####copy_res_to_mermaid("Pictures/GoToTitle.bmp" "images/1023.bmp")
copy_res_to_mermaid("Pictures/BasementTileSrc.bmp" "images/1202.bmp")
copy_res_to_mermaid("Pictures/DirtTileSrc.bmp" "images/1211.bmp")
copy_res_to_mermaid("Pictures/StratosphereTileSrc.bmp" "images/1216.bmp")
copy_res_to_mermaid("Pictures/StarsTileSrc.bmp" "images/1217.bmp")
copy_res_to_mermaid("Pictures/Letters.bmp" "images/1988.bmp")
copy_res_to_mermaid("Pictures/PagesMask.bmp" "images/1989.bmp")
copy_res_to_mermaid("Pictures/Pages.bmp" "images/1990.bmp")
copy_res_to_mermaid("Pictures/BannerPageBottomMask.bmp" "images/1991.bmp")
copy_res_to_mermaid("Pictures/BannerPageBottom.bmp" "images/1992.bmp")
copy_res_to_mermaid("Pictures/BannerPageTop.bmp" "images/1993.bmp")
copy_res_to_mermaid("Pictures/HighScores.bmp" "images/1994.bmp")
copy_res_to_mermaid("Pictures/Star.bmp" "images/1995.bmp")
copy_res_to_mermaid("Pictures/Badge.bmp" "images/1996.bmp")
copy_res_to_mermaid("Pictures/Scoreboard.bmp" "images/1997.bmp")
copy_res_to_mermaid("Pictures/HighScoresMask.bmp" "images/1998.bmp")
copy_res_to_mermaid("Pictures/Support.bmp" "images/1999.bmp")
copy_res_to_mermaid("Pictures/SimpleRoom.bmp" "images/2000.bmp")
copy_res_to_mermaid("Pictures/PaneledRoom.bmp" "images/2001.bmp")
copy_res_to_mermaid("Pictures/Basement.bmp" "images/2002.bmp")
copy_res_to_mermaid("Pictures/ChildsRoom.bmp" "images/2003.bmp")
copy_res_to_mermaid("Pictures/AsianRoom.bmp" "images/2004.bmp")
copy_res_to_mermaid("Pictures/UnfinishedRoom.bmp" "images/2005.bmp")
copy_res_to_mermaid("Pictures/SwingersRoom.bmp" "images/2006.bmp")
copy_res_to_mermaid("Pictures/Bathroom.bmp" "images/2007.bmp")
copy_res_to_mermaid("Pictures/Library.bmp" "images/2008.bmp")
copy_res_to_mermaid("Pictures/Garden.bmp" "images/2009.bmp")
copy_res_to_mermaid("Pictures/Skywalk.bmp" "images/2010.bmp")
copy_res_to_mermaid("Pictures/Dirt.bmp" "images/2011.bmp")
copy_res_to_mermaid("Pictures/Meadow.bmp" "images/2012.bmp")
copy_res_to_mermaid("Pictures/Field.bmp" "images/2013.bmp")
copy_res_to_mermaid("Pictures/Roof.bmp" "images/2014.bmp")
copy_res_to_mermaid("Pictures/Sky.bmp" "images/2015.bmp")
copy_res_to_mermaid("Pictures/Stratosphere.bmp" "images/2016.bmp")
copy_res_to_mermaid("Pictures/Stars.bmp" "images/2017.bmp")
copy_res_to_mermaid("Pictures/MailboxRightMask.bmp" "images/3903.bmp")
copy_res_to_mermaid("Pictures/MailboxLeftMask.bmp" "images/3904.bmp")
copy_res_to_mermaid("Pictures/TVMask.bmp" "images/3912.bmp")
copy_res_to_mermaid("Pictures/VCRMask.bmp" "images/3913.bmp")
copy_res_to_mermaid("Pictures/StereoMask.bmp" "images/3914.bmp")
copy_res_to_mermaid("Pictures/MicrowaveMask.bmp" "images/3915.bmp")
copy_res_to_mermaid("Pictures/CloudMask.bmp" "images/3921.bmp")
copy_res_to_mermaid("Pictures/CobwebMask.bmp" "images/3927.bmp")
copy_res_to_mermaid("Pictures/ManholeThruFloor.bmp" "images/3957.bmp")
copy_res_to_mermaid("Pictures/Cobweb.bmp" "images/3958.bmp")
copy_res_to_mermaid("Pictures/FlowerBox.bmp" "images/3959.bmp")
copy_res_to_mermaid("Pictures/Cinder.bmp" "images/3960.bmp")
copy_res_to_mermaid("Pictures/Chimes.bmp" "images/3961.bmp")
copy_res_to_mermaid("Pictures/Rug.bmp" "images/3962.bmp")
copy_res_to_mermaid("Pictures/GliderFoil2.bmp" "images/3963.bmp")
copy_res_to_mermaid("Pictures/Books.bmp" "images/3964.bmp")
copy_res_to_mermaid("Pictures/Cloud.bmp" "images/3965.bmp")
copy_res_to_mermaid("Pictures/Bulletin.bmp" "images/3966.bmp")
copy_res_to_mermaid("Pictures/Manhole.bmp" "images/3967.bmp")
copy_res_to_mermaid("Pictures/Vase2.bmp" "images/3968.bmp")
copy_res_to_mermaid("Pictures/Vase1.bmp" "images/3969.bmp")
copy_res_to_mermaid("Pictures/Calendar.bmp" "images/3970.bmp")
copy_res_to_mermaid("Pictures/Microwave.bmp" "images/3971.bmp")
copy_res_to_mermaid("Pictures/Bear.bmp" "images/3972.bmp")
copy_res_to_mermaid("Pictures/Fireplace.bmp" "images/3973.bmp")
copy_res_to_mermaid("Pictures/Glider2.bmp" "images/3974.bmp")
copy_res_to_mermaid("Pictures/Ozma.bmp" "images/3975.bmp")
copy_res_to_mermaid("Pictures/GliderFoil.bmp" "images/3976.bmp")
copy_res_to_mermaid("Pictures/WindowExRight.bmp" "images/3977.bmp")
copy_res_to_mermaid("Pictures/WindowExLeft.bmp" "images/3978.bmp")
copy_res_to_mermaid("Pictures/WindowInRight.bmp" "images/3979.bmp")
copy_res_to_mermaid("Pictures/WindowInLeft.bmp" "images/3980.bmp")
copy_res_to_mermaid("Pictures/DoorExLeft.bmp" "images/3981.bmp")
copy_res_to_mermaid("Pictures/DoorExRight.bmp" "images/3982.bmp")
copy_res_to_mermaid("Pictures/DoorInRight.bmp" "images/3983.bmp")
copy_res_to_mermaid("Pictures/DoorInLeft.bmp" "images/3984.bmp")
copy_res_to_mermaid("Pictures/MailboxRight.bmp" "images/3985.bmp")
copy_res_to_mermaid("Pictures/MailboxLeft.bmp" "images/3986.bmp")
copy_res_to_mermaid("Pictures/Trunk.bmp" "images/3987.bmp")
copy_res_to_mermaid("Pictures/BBQ.bmp" "images/3988.bmp")
copy_res_to_mermaid("Pictures/Stereo.bmp" "images/3989.bmp")
copy_res_to_mermaid("Pictures/VCR.bmp" "images/3990.bmp")
copy_res_to_mermaid("Pictures/Guitar.bmp" "images/3991.bmp")
copy_res_to_mermaid("Pictures/TV.bmp" "images/3992.bmp")
copy_res_to_mermaid("Pictures/DecoLamp.bmp" "images/3993.bmp")
copy_res_to_mermaid("Pictures/HipLamp.bmp" "images/3994.bmp")
copy_res_to_mermaid("Pictures/FilingCabinet.bmp" "images/3995.bmp")
copy_res_to_mermaid("Pictures/DownStairs.bmp" "images/3996.bmp")
copy_res_to_mermaid("Pictures/UpStairs.bmp" "images/3997.bmp")
copy_res_to_mermaid("Pictures/Shadow.bmp" "images/3998.bmp")
copy_res_to_mermaid("Pictures/Glider.bmp" "images/3999.bmp")
copy_res_to_mermaid("Pictures/Blower.bmp" "images/4000.bmp")
copy_res_to_mermaid("Pictures/Furniture.bmp" "images/4001.bmp")
copy_res_to_mermaid("Pictures/Bonus.bmp" "images/4002.bmp")
copy_res_to_mermaid("Pictures/Switch.bmp" "images/4003.bmp")
copy_res_to_mermaid("Pictures/Light.bmp" "images/4004.bmp")
copy_res_to_mermaid("Pictures/Appliance.bmp" "images/4005.bmp")
copy_res_to_mermaid("Pictures/Points.bmp" "images/4006.bmp")
copy_res_to_mermaid("Pictures/RubberBands.bmp" "images/4007.bmp")
copy_res_to_mermaid("Pictures/Transport.bmp" "images/4008.bmp")
copy_res_to_mermaid("Pictures/Toast.bmp" "images/4009.bmp")
copy_res_to_mermaid("Pictures/Shredded.bmp" "images/4010.bmp")
copy_res_to_mermaid("Pictures/Balloon.bmp" "images/4011.bmp")
copy_res_to_mermaid("Pictures/Copter.bmp" "images/4012.bmp")
copy_res_to_mermaid("Pictures/Dart.bmp" "images/4013.bmp")
copy_res_to_mermaid("Pictures/Ball.bmp" "images/4014.bmp")
copy_res_to_mermaid("Pictures/Drip.bmp" "images/4015.bmp")
copy_res_to_mermaid("Pictures/Enemy.bmp" "images/4016.bmp")
copy_res_to_mermaid("Pictures/Fish.bmp" "images/4017.bmp")
copy_res_to_mermaid("Pictures/Clutter.bmp" "images/4018.bmp")
copy_res_to_mermaid("Pictures/ShadowMask.bmp" "images/4998.bmp")
copy_res_to_mermaid("Pictures/GliderMask.bmp" "images/4999.bmp")
copy_res_to_mermaid("Pictures/BlowerMask.bmp" "images/5000.bmp")
copy_res_to_mermaid("Pictures/FurnitureMask.bmp" "images/5001.bmp")
copy_res_to_mermaid("Pictures/BonusMask.bmp" "images/5002.bmp")
copy_res_to_mermaid("Pictures/LightMask.bmp" "images/5004.bmp")
copy_res_to_mermaid("Pictures/ApplianceMask.bmp" "images/5005.bmp")
copy_res_to_mermaid("Pictures/PointsMask.bmp" "images/5006.bmp")
copy_res_to_mermaid("Pictures/RubberBandsMask.bmp" "images/5007.bmp")
copy_res_to_mermaid("Pictures/TransportMask.bmp" "images/5008.bmp")
copy_res_to_mermaid("Pictures/ToastMask.bmp" "images/5009.bmp")
copy_res_to_mermaid("Pictures/ShreddedMask.bmp" "images/5010.bmp")
copy_res_to_mermaid("Pictures/BalloonMask.bmp" "images/5011.bmp")
copy_res_to_mermaid("Pictures/CopterMask.bmp" "images/5012.bmp")
copy_res_to_mermaid("Pictures/DartMask.bmp" "images/5013.bmp")
copy_res_to_mermaid("Pictures/BallMask.bmp" "images/5014.bmp")
copy_res_to_mermaid("Pictures/DripMask.bmp" "images/5015.bmp")
copy_res_to_mermaid("Pictures/EnemyMask.bmp" "images/5016.bmp")
copy_res_to_mermaid("Pictures/FishMask.bmp" "images/5017.bmp")
copy_res_to_mermaid("Pictures/ClutterMask.bmp" "images/5018.bmp")
copy_res_to_mermaid("Pictures/CustomPictFallback.bmp" "images/10000.bmp")

#
# Sound files
#

copy_res_to_mermaid("Sounds/HitWall.wav" "sounds/1000.wav")
copy_res_to_mermaid("Sounds/FadeIn.wav" "sounds/1001.wav")
copy_res_to_mermaid("Sounds/FadeOut.wav" "sounds/1002.wav")
copy_res_to_mermaid("Sounds/Beeps.wav" "sounds/1003.wav")
copy_res_to_mermaid("Sounds/Buzzer.wav" "sounds/1004.wav")
copy_res_to_mermaid("Sounds/Ding.wav" "sounds/1005.wav")
copy_res_to_mermaid("Sounds/Energize.wav" "sounds/1006.wav")
copy_res_to_mermaid("Sounds/Follow.wav" "sounds/1007.wav")
copy_res_to_mermaid("Sounds/Microwaved.wav" "sounds/1008.wav")
copy_res_to_mermaid("Sounds/Switch.wav" "sounds/1009.wav")
copy_res_to_mermaid("Sounds/Bird.wav" "sounds/1010.wav")
copy_res_to_mermaid("Sounds/Cuckoo.wav" "sounds/1011.wav")
copy_res_to_mermaid("Sounds/Tik.wav" "sounds/1012.wav")
copy_res_to_mermaid("Sounds/Tok.wav" "sounds/1013.wav")
copy_res_to_mermaid("Sounds/BlowerOn.wav" "sounds/1014.wav")
copy_res_to_mermaid("Sounds/BlowerOff.wav" "sounds/1015.wav")
copy_res_to_mermaid("Sounds/CaughtFire.wav" "sounds/1016.wav")
copy_res_to_mermaid("Sounds/ScoreTik.wav" "sounds/1017.wav")
copy_res_to_mermaid("Sounds/Thrust.wav" "sounds/1018.wav")
copy_res_to_mermaid("Sounds/Fizzle.wav" "sounds/1019.wav")
copy_res_to_mermaid("Sounds/FireBand.wav" "sounds/1020.wav")
copy_res_to_mermaid("Sounds/BandRebound.wav" "sounds/1021.wav")
copy_res_to_mermaid("Sounds/GreaseSpill.wav" "sounds/1022.wav")
copy_res_to_mermaid("Sounds/Chord.wav" "sounds/1023.wav")
copy_res_to_mermaid("Sounds/VCR.wav" "sounds/1024.wav")
copy_res_to_mermaid("Sounds/FoilHit.wav" "sounds/1025.wav")
copy_res_to_mermaid("Sounds/Shred.wav" "sounds/1026.wav")
copy_res_to_mermaid("Sounds/ToastLaunch.wav" "sounds/1027.wav")
copy_res_to_mermaid("Sounds/ToastLand.wav" "sounds/1028.wav")
copy_res_to_mermaid("Sounds/MacOn.wav" "sounds/1029.wav")
copy_res_to_mermaid("Sounds/MacBeep.wav" "sounds/1030.wav")
copy_res_to_mermaid("Sounds/MacOff.wav" "sounds/1031.wav")
copy_res_to_mermaid("Sounds/TVOn.wav" "sounds/1032.wav")
copy_res_to_mermaid("Sounds/TVOff.wav" "sounds/1033.wav")
copy_res_to_mermaid("Sounds/Coffee.wav" "sounds/1034.wav")
copy_res_to_mermaid("Sounds/Mystic.wav" "sounds/1035.wav")
copy_res_to_mermaid("Sounds/Zap.wav" "sounds/1036.wav")
copy_res_to_mermaid("Sounds/Pop.wav" "sounds/1037.wav")
copy_res_to_mermaid("Sounds/EnemyIn.wav" "sounds/1038.wav")
copy_res_to_mermaid("Sounds/EnemyOut.wav" "sounds/1039.wav")
copy_res_to_mermaid("Sounds/PaperCrunch.wav" "sounds/1040.wav")
copy_res_to_mermaid("Sounds/Bounce.wav" "sounds/1041.wav")
copy_res_to_mermaid("Sounds/Drip.wav" "sounds/1042.wav")
copy_res_to_mermaid("Sounds/Drop.wav" "sounds/1043.wav")
copy_res_to_mermaid("Sounds/FishOut.wav" "sounds/1044.wav")
copy_res_to_mermaid("Sounds/FishIn.wav" "sounds/1045.wav")
copy_res_to_mermaid("Sounds/DontExit.wav" "sounds/1046.wav")
copy_res_to_mermaid("Sounds/Sizzle.wav" "sounds/1047.wav")
copy_res_to_mermaid("Sounds/Paper1.wav" "sounds/1048.wav")
copy_res_to_mermaid("Sounds/Paper2.wav" "sounds/1049.wav")
copy_res_to_mermaid("Sounds/Paper3.wav" "sounds/1050.wav")
copy_res_to_mermaid("Sounds/Paper4.wav" "sounds/1051.wav")
copy_res_to_mermaid("Sounds/Typing.wav" "sounds/1052.wav")
copy_res_to_mermaid("Sounds/Carriage.wav" "sounds/1053.wav")
copy_res_to_mermaid("Sounds/Chord2.wav" "sounds/1054.wav")
copy_res_to_mermaid("Sounds/PhoneRing.wav" "sounds/1055.wav")
copy_res_to_mermaid("Sounds/Chime1.wav" "sounds/1056.wav")
copy_res_to_mermaid("Sounds/Chime2.wav" "sounds/1057.wav")
copy_res_to_mermaid("Sounds/WebTwang.wav" "sounds/1058.wav")
copy_res_to_mermaid("Sounds/TransOut.wav" "sounds/1059.wav")
copy_res_to_mermaid("Sounds/TransIn.wav" "sounds/1060.wav")
copy_res_to_mermaid("Sounds/Bonus.wav" "sounds/1061.wav")
copy_res_to_mermaid("Sounds/Hiss.wav" "sounds/1062.wav")

#
# Music files
#

copy_res_to_mermaid("Music/Refrain1.wav" "sounds/2000.wav")
copy_res_to_mermaid("Music/Refrain2.wav" "sounds/2001.wav")
copy_res_to_mermaid("Music/Refrain3.wav" "sounds/2002.wav")
copy_res_to_mermaid("Music/Refrain4.wav" "sounds/2003.wav")
copy_res_to_mermaid("Music/Chorus.wav" "sounds/2004.wav")
copy_res_to_mermaid("Music/RefrainSparse1.wav" "sounds/2005.wav")
copy_res_to_mermaid("Music/RefrainSparse2.wav" "sounds/2006.wav")

#
# Create the Mermaid.dat zip file
#

add_custom_command(
  OUTPUT Mermaid.dat
  COMMAND "${CMAKE_COMMAND}" -E
    tar c "${MERMAID_OUTPUT}" --format=zip "--files-from=${MERMAID_FILELIST}"
  WORKING_DIRECTORY "${MERMAID_DIRECTORY}"
  DEPENDS "${MERMAID_DEPENDENCIES}"
)

#
# Make sure Mermaid.dat is next to the executable, if it isn't already
#

add_custom_command(
  TARGET gliderpro POST_BUILD
  COMMAND "${CMAKE_COMMAND}" -E
    copy "${MERMAID_OUTPUT}" "$<TARGET_FILE_DIR:gliderpro>/Mermaid.dat"
)
