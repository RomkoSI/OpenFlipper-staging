# OpenFlipper Staging Repository

This repository contains Plugins and Objecttypes which are currently not in a 
releasable state for the Free branch. They might need more cleanup and testing 
or have other flaws.


## Using the Staging repository


* Clone the free branch to get OpenFlipper

    git clone http://www.openflipper.org:9000/OpenFlipper-Free/OpenFlipper-Free.git OpenFlipper-Staging
  
* Enter the directory and clone the Staging repository into it:

    git clone http://www.openflipper.org:9000/OpenFlipper-Free/OpenFlipper-Staging.git Package-Staging

    `Note`: You need to name the checkout folder Package-something to get it picked up by the build System
        
* Build as usual.