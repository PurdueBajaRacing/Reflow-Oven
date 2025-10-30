# Reflow Oven
With the addition of SMD components to our PCBs, our Baja Racing team decided that we needed a reflow oven to easily solder our boards whenever needed. Since even a cheap reflow oven is ~$200, it's far better financially to just make your own. Not only is it a fun project to tackle, but it'll also allow us to have a lot more control over how we assemble our boards. 
# Materials 
There isn't a very strict materials list of stuff you need to have to make this oven; it's encouraged to use what you have and buy as little as possible. 
1. Toaster oven - Buy this used off of Facebook Marketplace, they go for $10-$20 used. Try to get one with as small a glass window as possible, and as few controls as possible.
2. Extreme/High Heat JB Weld - You can find this stuff in the adhesives section of most hardware stores. Try to get a syringe style or tip for easier application.
3. Insulation - Get either fiberglass, rockwool, or ceramic insulation. Only get a small 16"x48" roll of this stuff, you shouldn't need too much. Try to check how heat-resistant it is, making sure there isn't any sort of combustion warning.
4. Type K thermocouple & amplifier
5. Solid-state relay - Do not skimp on this component; make sure to get a beefy SSR rated for at least 120V AC. A decent SSR will go up much higher than this.
6. Raspberry Pico - While you could use any microcontroller for this project, our code is for a Raspberry Pico
7. Jumper wires - Always good to have
8. AC/DC rectifier - You can steal this out of a random power brick you have, just make sure the output DC voltage matches whatever your microcontroller needs.
9. TFT screen - For our oven, we used a 2.2 TFT SPI screen, but you can use whatever size you'd like
10. Scrap metal (OPTIONAL) - You may want to create mounting brackets, or cover up large holes in your oven with this
11. Outlet w/ fuze 
# The Build
Start by cleaning the oven thoroughly with isopropyl alcohol and some towels. Get all the food bits, stains, and anything else in there you don't want. Once that's done, you can do a test run if you'd like to check that all the heating elements work as intended, and make sure it cools down all the way before proceeding with the next steps. Put a big label on the handle of the oven, marked "NOT FOR FOOD", since eating food cooked in the same oven with JB weld and solder paste will almost definitely be bad for you. Begin slowly removing all of the fasteners holding the cover onto the oven; most of the fasteners should be on the back and bottom of the oven. I used an impact driver with a Phillips head to speed up this process significantly. Once you're able to remove the oven cover, the inside of the oven should look like this (more or less):
<img width="540" height="720" alt="image" src="https://github.com/user-attachments/assets/1475d6f6-00af-43a9-88cd-56e034ee3cef" />
