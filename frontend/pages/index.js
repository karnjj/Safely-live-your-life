import { useEffect, useState } from "react";

import { WiThermometer, WiHumidity, WiSmog } from "react-icons/wi";
import { FaLightbulb } from "react-icons/fa";

import axios from "axios";

const clientId = "a8dfc6a7-1799-46a1-b799-55735a2003ff";
const token = "hM5wWXzpJjzEmp42M4FKcYbjWw7hqfaX";
const API_HOST = "https://api.netpie.io/v2";

export default function Home() {
  const [pm, setPm] = useState("Calibrating");
  const [light, setLight] = useState("ON");
  const [data, setData] = useState({});

  useEffect(() => {
    const pollingData = setInterval(async () => {
      const { data } = await axios
        .get(API_HOST + "/device/shadow/data", {
          headers: {
            Authorization: `Device ${clientId}:${token}`,
          },
        })
        .then((res) => res.data);
      setData(data);

      if (data.lumi < 3500) {
        setLight("ON");
      } else {
        setLight("OFF");
      }
      if (data.pm < 0.007) {
        setPm("Excellent");
      } else if (data.pm >= 0.007 && data.pm < 0.015) {
        setPm("Fine");
      } else if (data.pm >= 0.015 && data.pm < 0.03) {
        setPm("Moderate");
      } else if (data.pm >= 0.03 && data.pm < 0.055) {
        setPm("Poor");
      } else if (data.pm >= 0.055 && data.pm < 0.011) {
        setPm("Very Poor");
      } else {
        setPm("Severe");
      }
    }, 1000);
    return () => clearInterval(pollingData);
  }, []);

  return (
    <div className="container mx-auto">
      <RoomTitle />
      <div className="flex flex-row justify-around pt-8">
        <Card>
          <div className="flex justify-center">
            <button
              type="button"
              style={{
                color: "rgb(15, 143, 255)",
                backgroundColor: "rgb(173, 217, 255)",
              }}
              className="text-2xl opacity-0.9 rounded-full  p-4 hover:drop-shadow-xl"
            >
              {<WiThermometer />}
            </button>
          </div>
          <div className="flex mt-3 justify-center">
            <div className="text-lg font-semibold">{data.temp}</div>
            <div className="text-sm text-gray-700 mt-1">{" °C"}</div>
          </div>
          <div className="text-sm text-gray-400  mt-1 text-center">
            {"temperature"}
          </div>
        </Card>
        <Card>
          <div className="flex justify-center">
            <button
              type="button"
              style={{
                color: "rgb(247, 99, 114)",
                backgroundColor: "rgb(250, 192, 199)",
              }}
              className="text-2xl opacity-0.9 rounded-full  p-4 hover:drop-shadow-xl"
            >
              {<WiHumidity />}
            </button>
          </div>
          <div className="flex mt-3 justify-center">
            <div className="text-lg font-semibold">{data.humi}</div>
            <div className="text-sm text-gray-700 mt-1">{" %"}</div>
          </div>
          <div className="text-sm text-gray-400 mt-1 text-center">
            {"Humidity"}
          </div>
        </Card>
        <Card>
          <div className="flex justify-center">
            <button
              type="button"
              style={{
                color: "rgb(255, 208, 20)",
                backgroundColor: "rgb(255, 238, 168)",
              }}
              className="text-2xl opacity-0.9 rounded-full p-4 hover:drop-shadow-xl"
            >
              {<FaLightbulb />}
            </button>
          </div>
          <div className="flex mt-3 justify-center">
            <div className="text-lg font-semibold">{light}</div>
          </div>
          <div className="text-sm text-gray-400 mt-1 text-center">
            {"Luminosity"}
          </div>
        </Card>
        <Card>
          <div className="flex justify-center">
            <button
              type="button"
              style={{
                color: "rgb(77, 77, 77)",
                backgroundColor: "rgb(150, 150, 150)",
              }}
              className="text-2xl opacity-0.9 rounded-full  p-4 hover:drop-shadow-xl"
            >
              {<WiSmog />}
            </button>
          </div>
          <div className="flex mt-3 justify-center">
            <div className="text-lg font-semibold">
              {(data.pm * 1000).toFixed(2)}
            </div>
            <div className="text-sm text-gray-700 mt-1">{" μg/m3"}</div>
          </div>
          <div className="text-lg text-gray-900  mt-1 text-center">{pm}</div>
          <div className="text-sm text-gray-400  mt-1 text-center">
            {"PM 2.5"}
          </div>
        </Card>
      </div>
    </div>
  );
}

const RoomTitle = () => {
  return (
    <div className="flex flex-wrap lg:flex-nowrap justify-center ">
      <div className="bg-white dark:text-gray-200 dark:bg-secondary-dark-bg h-44 rounded-xl w-full lg:w-80 p-8 pt-9 m-3 bg-hero-pattern bg-no-repeat bg-cover bg-center">
        <div className="flex justify-between items-center">
          <div>
            <p className="font-bold text-gray-400 text-4xl">Room 1</p>
          </div>
        </div>
      </div>
    </div>
  );
};

const Card = ({ children }) => {
  return (
    <div className="grow">
      <div className="">{children}</div>
    </div>
  );
};
