#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>
#include "appconfig.h"
#include "onedrive.h"

using namespace OneDrive;

int main()
{
	bool success = true;

	try {
		std::unique_ptr<CAppConfig> config = std::make_unique<CAppConfig>();
		COneDrive oneDrive(std::move(config));

		CDrive drive = oneDrive.drive();

		std::cout << "Drive ID: " << drive.id() << std::endl;
		std::cout << "Quota remaining: " << drive.quota().remaining() << std::endl;

		std::list<CDrive> drives;

		oneDrive.drives(drives);

		std::cout << "Found " << drives.size() << " drive(s)" << std::endl;

		for (auto &&i : drives) {
			std::cout << "\tDrive ID: " << i.id() << std::endl;
			std::cout << "\tOwner: " << i.owner().displayName() << std::endl;
			std::cout << "\tQuota remaining: " << i.quota().remaining() << std::endl;
		}

		std::cout << std::endl;

		std::list<CDriveItem> driveItems;

		oneDrive.listChildren(driveItems);

		std::cout << "Found " << driveItems.size() << " drive item(s)" << std::endl;

		for (auto &&i : driveItems) {
			std::cout << "\tItem ID: " << i.id() << std::endl;
			std::cout << "\tItem name: " << i.name() << std::endl;
			std::cout << "\tItem size: " << i.size() << std::endl;
			std::cout << "\tItem create time: " << i.createTime() << std::endl;
			std::cout << "\tItem modified time: " << i.modifiedTime() << std::endl;

			if (i.type() == OneDrive::CDriveItem::DRIVE_ITEM_FOLDER)
				std::cout << "\tItem type: folder" << std::endl;
			else
				std::cout << "\tItem type: file, sha1: " << i.hash() << std::endl;
		}

		std::cout << std::endl;

		bool downloaded = false;

		for (auto &&i : driveItems) {
			std::cout << "\tItem name: " << i.name() << std::endl;

			std::list<CDriveItem> childDriveItems;

			oneDrive.listChildren(i, childDriveItems);

			for (auto &&j : childDriveItems) {
				std::cout << "\t\tItem name: " << j.name() << std::endl;

				if (j.type() == OneDrive::CDriveItem::DRIVE_ITEM_FOLDER)
					std::cout << "\t\tItem type: folder" << std::endl;
				else {
					std::cout << "\t\tItem type: file, sha1: " << j.hash() << std::endl;

					if (!downloaded) {
						downloaded = true;

						std::ofstream f(j.name(), std::ios::binary);

						oneDrive.download(j, f);

					}
				}
			}
		}
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << std::endl;
		success = false;
	} catch (...) {
		std::cerr << "Error: un unknown exception has occurred" << std::endl;
		success = false;
	}

	return ( success ? 0 : 1 );
}
