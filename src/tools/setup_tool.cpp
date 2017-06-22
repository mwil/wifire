//
// Copyright 2009-2010 Ettus Research LLC
// Copyright 2009-2011 Disco Labs, TU Kaiserslautern
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

/*
 * Tool to setup USRP2 over UHD for Wifire.
 *
 * Editor: Matthias Schaefer (m_schae3@cs.uni-kl.de)
 */

#ifdef TX
#define TX_FREQ
#define TX_GAIN
#define TX_SR
#endif

#ifdef RX
#define RX_FREQ
#define RX_GAIN
#define RX_SR
#endif

#include <uhd/utils/thread_priority.hpp>
//#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/static.hpp>
#include <uhd/usrp/single_usrp.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/thread_time.hpp> //system time
#include <boost/math/special_functions/round.hpp>
#include <boost/format.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <complex>
#include <cmath>

namespace po = boost::program_options;

int main (int argc, char *argv[]){

    //variables to be set by po
    std::string args;
    double tx_rate, rx_rate, freq;
    float gain;

    //setup the program options
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "help message")
        ("args", po::value<std::string>(&args)->default_value(""), "simple uhd device address args")
        ("tx_rate", po::value<double>(&tx_rate)->default_value(4e6), "rate of outgoing samples")
        ("rx_rate", po::value<double>(&rx_rate)->default_value(4e6), "rate of incoming samples")
        ("freq", po::value<double>(&freq)->default_value(2.48e9), "rf center frequency in Hz")
        ("gain", po::value<float>(&gain)->default_value(float(61)), "gain for the RF chain")
    ;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    //print the help message
    if (vm.count("help")){
        std::cout << boost::format("Wifire Setup Tool %s") % desc << std::endl;
        return ~0;
    }

    //create a usrp device
    std::cout << std::endl;
    std::cout << boost::format("Creating the usrp device with: %s...") % args << std::endl;
    uhd::usrp::single_usrp::sptr sdev = uhd::usrp::single_usrp::make(args);

    //uhd::device::sptr dev = sdev->get_device();

    std::cout << boost::format("Using Device: %s") % sdev->get_pp_string() << std::endl;

//    std::cout << boost::format("Setting Antennas...") << std::endl;
//    sdev->set_rx_antenna("J2");
//    sdev->set_tx_antenna("J1");
//    std::cout << boost::format("done (tx: %s/rx: %s).") % sdev->get_tx_antenna() % sdev->get_rx_antenna() << std::endl << std::endl;


#ifdef TX_SR
    //set the tx sample rate
    std::cout << boost::format("Setting TX Rate: %f Msps...") % (tx_rate/1e6) << std::endl;
    sdev->set_tx_rate(tx_rate);
    std::cout << boost::format("Actual TX Rate: %f Msps...") % (sdev->get_tx_rate()/1e6) << std::endl;
#endif

#ifdef TX_FREQ
    //set the tx center frequency
    std::cout << boost::format("Setting TX Freq: %f Mhz...") % (freq/1e6) << std::endl;
    sdev->set_tx_freq(freq);
    std::cout << boost::format("Actual TX Freq: %f Mhz...") % (sdev->get_tx_freq()/1e6) << std::endl << std::endl;
#endif

#ifdef TX_GAIN
    //set the tx rf gain
    std::cout << boost::format("Setting TX Gain: %f dB...") % gain << std::endl;
    sdev->set_tx_gain(gain > 20 ? 20 : gain);
    std::cout << boost::format("Actual TX Gain: %f dB...") % sdev->get_tx_gain() << std::endl << std::endl;
#endif


#ifdef RX_SR
    //set the tx sample rate
    std::cout << boost::format("Setting RX Rate: %f Msps...") % (rx_rate/1e6) << std::endl;
    sdev->set_rx_rate(rx_rate);
    std::cout << boost::format("Actual RX Rate: %f Msps...") % (sdev->get_rx_rate()/1e6) << std::endl;
#endif

#ifdef RX_FREQ
    //set the rx center frequency
    std::cout << boost::format("Setting RX Freq: %f Mhz...") % (freq/1e6) << std::endl;
    sdev->set_rx_freq(freq);
    std::cout << boost::format("Actual RX Freq: %f Mhz") % (sdev->get_rx_freq()/1e6) << std::endl << std::endl;
#endif

#ifdef RX_GAIN
    //set the rx rf gain
    std::cout << boost::format("Setting RX Gain: %f dB...") % gain << std::endl;
    sdev->set_rx_gain(gain);
    std::cout << boost::format("Actual RX Gain: %f dB") % sdev->get_rx_gain() << std::endl << std::endl;
#endif

#ifdef RX_X
    std::cout << boost::format("Setting STREAN_MODE_START_CONTINUOUS...") << std::endl;
    sdev->issue_stream_cmd(uhd::stream_cmd_t::STREAM_MODE_START_CONTINUOUS);
#endif

    //finished
    std::cout << std::endl << "Done!" << std::endl << std::endl;

    std::exit(0);
}
