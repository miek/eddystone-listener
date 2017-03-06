#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <cerrno>
#include <array>
#include <iomanip>
#include <vector>
#include <boost/optional.hpp>

#include <signal.h>

#include <stdexcept>

#include <blepp/logging.h>
#include <blepp/pretty_printers.h>
#include <blepp/blestatemachine.h> //for UUID. FIXME mofo
#include <blepp/lescan.h>

using namespace std;
using namespace BLEPP;

std::string print_vector(std::vector<uint8_t>::const_iterator begin, std::vector<uint8_t>::const_iterator end)
{
    ostringstream stream;
    for (auto i = begin; i != end; i++)
        stream << std::hex << (int)*i << ' ';
    return stream.str();
}

bool parse_eid_frame(std::vector<uint8_t> data, AdvertisingResponse ad)
{
    cout << "EID frame from " << ad.address << ": " << print_vector(data.begin(), data.end()) << endl;
    return true;
}

bool parse_tlm_frame(std::vector<uint8_t> data, AdvertisingResponse ad)
{
    cout << "TLM frame from " << ad.address << ": " << print_vector(data.begin(), data.end()) << endl;
    return true;
}

void parse_eddystone_frame(AdvertisingResponse ad)
{
    auto data = ad.unparsed_data_with_types[0];
    if (ad.unparsed_data_with_types.size() <= 0)
        return;

    if (data.size() < 4)
        return;

    // 'Service Data'
    if (data[0] != 0x16)
        return;

    // Check for Eddystone UUID
    if (data[1] != 0xAA || data[2] != 0xFE)
        return;

    bool success = false;
    std::vector<uint8_t> frame_data(data.begin() + 3, data.end());
    switch (frame_data[0]) {
        case 0x20:
            success = parse_tlm_frame(frame_data, ad);
            break;
        case 0x30:
            success = parse_eid_frame(frame_data, ad);
            break;
        default:
            cout << "UNK frame from " << ad.address << ": " << print_vector(frame_data.begin(), frame_data.end()) << endl;
            success = true;
    }

    if (!success) {
        // Debug printout
        cout << "Parse failed: " << print_vector(data.begin(), data.end()) << endl;
    }
}

void catch_function(int)
{
	cerr << "\nInterrupted!\n";
}

int main(int argc, char** argv)
{
	HCIScanner::ScanType type = HCIScanner::ScanType::Active;

	log_level = LogLevels::Warning;
	HCIScanner scanner(true, HCIScanner::FilterDuplicates::Off, type);
	
	//Catch the interrupt signal. If the scanner is not 
	//cleaned up properly, then it doesn't reset the HCI state.
	signal(SIGINT, catch_function);

	int i=0;
	while (1) {
		

		//Check to see if there's anything to read from the HCI
		//and wait if there's not.
		struct timeval timeout;     
		timeout.tv_sec = 0;     
		timeout.tv_usec = 300000;

		fd_set fds;
		FD_ZERO(&fds);
		FD_SET(scanner.get_fd(), &fds);
		int err = select(scanner.get_fd()+1, &fds, NULL, NULL,  &timeout);
		
		//Interrupted, so quit and clean up properly.
		if(err < 0 && errno == EINTR)	
			break;
		
		if(FD_ISSET(scanner.get_fd(), &fds))
		{
			//Only read id there's something to read
			vector<AdvertisingResponse> ads = scanner.get_advertisements();

			for(const auto& ad: ads)
			{
                parse_eddystone_frame(ad);
			}
		}
		i++;
	}
}
