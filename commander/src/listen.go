package commander

import (
	"commander/src/packet"
	"commander/src/ws"
	"encoding/json"
	"errors"
	"io"
	"log"
	"os"

	"github.com/google/gousb"
	"github.com/gorilla/websocket"
)

func sendResponse(hub *ws.Hub, buf []byte) {
	packet_rx := packet.Parse(buf)
	if packet_rx == nil {
		return
	}
	res := ws.Response{Event: int32(packet_rx.Code), Data: packet_rx.Payload}
	if packet_rx.Code == packet.CodeErr {
		client := hub.Clients[packet_rx.ID]
		if client != nil {
			payload, _ := json.Marshal(res)
			client.Send <- payload
		}
		return
	}

	payload, _ := json.Marshal(res)
	hub.Broadcast <- payload
}

func Listen(hub *ws.Hub, writer io.Writer, reader io.Reader) {
	buf := make([]byte, packet.MaxPacketSizeIn+1)

	commands := [][]byte{
		{0xFF, packet.CodeDefaultLED, 0xFF}, // DEFAULT_LED_STATUS
		{0xFF, packet.CodeADCRead, 0x04},    // ADC_ONBOARD_STATUS
		{0xFF, packet.CodeTB6612FNG, 0xFF},  // METHODE_TB_6612_FNG_STATUS
	}

	for _, command := range commands {
		if _, err := writer.Write(command); err != nil {
			log.Fatalf("failed to write command: %v", err)
		}

		n, err := reader.Read(buf)

		if err != nil {
			switch {
			case errors.Is(err, gousb.TransferOverflow):
				log.Printf("gousb.TransferOverflow: %v", err)
				continue
			}
			for _, client := range hub.Clients {
				client.Conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseGoingAway, "Device disconnected"))
			}
			log.Printf("reader.Read failed: %v", err)
			os.Exit(0)
		}

		packet.Parse(buf[:n])
	}

	for {
		n, err := reader.Read(buf)
		if err != nil {
			switch {
			case errors.Is(err, gousb.TransferOverflow):
				log.Printf("gousb.TransferOverflow: %v", err)
				continue
			}
			for _, client := range hub.Clients {
				client.Conn.WriteMessage(websocket.CloseMessage, websocket.FormatCloseMessage(websocket.CloseGoingAway, "Device disconnected"))
			}
			log.Printf("reader.Read failed: %v", err)
			os.Exit(0)
		}

		if n == 0 {
			continue
		}

		sendResponse(hub, buf[:n])
	}
}
