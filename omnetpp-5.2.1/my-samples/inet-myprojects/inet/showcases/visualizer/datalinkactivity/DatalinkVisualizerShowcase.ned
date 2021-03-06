//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.showcases.visualizer.datalinkactivity;

import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.aodv.AODVRouter;
import inet.node.ethernet.Eth100M;
import inet.node.ethernet.EtherSwitch;
import inet.node.inet.AdhocHost;
import inet.node.inet.StandardHost;
import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;
import inet.visualizer.linklayer.DataLinkVisualizer;


//
// TODO documentation
//
network DataLinkVisualizerEnablingWiredShowcase
{
    @display("bgb=700,250");
    submodules:
        linkVisualizer: DataLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,150");
        }
        wiredSource: StandardHost {
            @display("p=300,110");
        }
        wiredDestination: StandardHost {
            @display("p=600,110");
        }
    connections:
        wiredSource.ethg++ <--> Eth100M <--> wiredDestination.ethg++;
}


//
// TODO documentation
//
network DataLinkVisualizerEnablingWirelessShowcase
{
    @display("bgb=700,250");
    submodules:
        linkVisualizer: DataLinkVisualizer {
            parameters:
                @display("p=100,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=100,120");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            parameters:
                @display("p=100,190");
        }
        wirelessSource: AdhocHost {
            @display("p=300,110");
        }
        wirelessDestination: AdhocHost {
            @display("p=600,110");
        }
}


//
// TODO documentation
//
network DataLinkVisualizerFilteringShowcase
{
    @display("bgb=900,600");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=80,50");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                @display("p=80,120");
        }
        etherSwitch1: EtherSwitch {
            @display("p=376.896,169.12");
        }
        etherSwitch3: EtherSwitch {
            @display("p=723.592,200.528");
        }
        source1: StandardHost {
            @display("p=206.568,50.736");
        }
        etherSwitch4: EtherSwitch {
            @display("p=484.408,283.88");
        }
        destination1: StandardHost {
            @display("p=556.888,485.616");
        }
        destination2: StandardHost {
            @display("p=798.488,343.072");
        }
        source2: StandardHost {
            @display("p=173.952,227.104");
        }
        etherSwitch2: EtherSwitch {
            @display("p=329.784,428.84");
        }
        host1: StandardHost {
            @display("p=233.144,506.152");
        }
        host2: StandardHost {
            @display("p=822.648,135.296");
        }
    connections:
        etherSwitch1.ethg++ <--> Eth100M <--> source1.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> etherSwitch4.ethg++;
        etherSwitch4.ethg++ <--> Eth100M <--> etherSwitch3.ethg++;
        etherSwitch1.ethg++ <--> Eth100M <--> source2.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> destination2.ethg++;
        etherSwitch4.ethg++ <--> Eth100M <--> etherSwitch2.ethg++;
        etherSwitch2.ethg++ <--> Eth100M <--> destination1.ethg++;
        etherSwitch3.ethg++ <--> Eth100M <--> host2.ethg++;
        etherSwitch2.ethg++ <--> Eth100M <--> host1.ethg++;
}

//
// TODO documentation
//
network DataLinkVisualizerMobileShowcase
{
    @display("bgb=620,500");
    submodules:
        visualizer: IntegratedVisualizer {
            @display("p=70,50");
        }
        configurator: IPv4NetworkConfigurator {
            @display("p=70,150");
        }
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=70,250");
        }
//        RREQVisualizer: DataLinkVisualizer {
//            @display("p=70,350");
//        }
//        RREPVisualizer: DataLinkVisualizer {
//            @display("p=70,450");
//        }
        destination: AODVRouter {
            @display("p=512.272,249.796");
        }
        source: AODVRouter {
            @display("p=187.664,207.952");
        }
        aodvRouter1: AODVRouter {
            @display("p=201.612,311.928");
        }
        aodvRouter2: AODVRouter {
            @display("p=465.356,158.5");
        }
        aodvRouter3: AODVRouter {
            @display("p=426.048,311.928");
        }
        aodvRouter4: AODVRouter {
            @display("p=400.688,88.76");
        }
        aodvRouter5: AODVRouter {
            @display("p=333.484,177.52");
        }
        aodvRouter6: AODVRouter {
            @display("p=323.34,262.476");
        }
        aodvRouter7: AODVRouter {
            @display("p=265.012,109.048");
        }
}

