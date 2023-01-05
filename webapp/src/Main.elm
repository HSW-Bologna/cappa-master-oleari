module Main exposing (Model, Msg(..), init, main, update, view)

import Browser
import Element as Ui
import Element.Background as UiBackground
import Element.Border as UiBorder
import Element.Font as UiFont
import Element.Input as UiInput
import File
import File.Select
import Html exposing (Html)
import Http



-- MAIN


main =
    Browser.document { init = init, update = update, view = view, subscriptions = subscriptions }



-- MODEL


type OtaState
    = OtaStateNone
    | OtaStateRunning
    | OtaStateSuccessful
    | OtaStateFailure


type alias Model =
    { file : Maybe File.File
    , otaState : OtaState
    }


init : () -> ( Model, Cmd msg )
init _ =
    ( { file = Nothing, otaState = OtaStateNone }, Cmd.none )


subscriptions : Model -> Sub Msg
subscriptions _ =
    Sub.none



-- UPDATE


type Msg
    = FirmwareRequested
    | FirmwareLoaded File.File
    | FirmwareUpdate File.File
    | OtaResult (Result Http.Error ())


update : Msg -> Model -> ( Model, Cmd Msg )
update msg model =
    case msg of
        FirmwareRequested ->
            ( model, File.Select.file [ "application/binary" ] FirmwareLoaded )

        FirmwareLoaded file ->
            ( { model | file = Just file }, Cmd.none )

        FirmwareUpdate file ->
            ( { model | otaState = OtaStateRunning }
            , Http.request
                { method = "PUT"
                , headers = []
                , url = "firmware_update"
                , body = Http.fileBody file
                , expect = Http.expectWhatever OtaResult
                , timeout = Just 1200000
                , tracker = Nothing
                }
            )

        OtaResult (Ok ()) ->
            ( { model | otaState = OtaStateSuccessful }, Cmd.none )

        OtaResult (Err _) ->
            ( { model | otaState = OtaStateFailure }, Cmd.none )



-- VIEW


themeColor =
    Ui.rgb255 64 138 245


darkerThemeColor =
    Ui.rgb255 34 108 215


darkColor =
    Ui.rgb255 72 72 72


textColor =
    Ui.rgb255 255 255 255


view : Model -> Browser.Document Msg
view model =
    let
        filename =
            case model.file of
                Nothing ->
                    "Nessun file selezionato"

                Just file ->
                    File.name file

        buttonStyle =
            [ Ui.width <| Ui.px 110
            , UiFont.center
            , UiBackground.color themeColor
            , Ui.padding 4
            , UiBorder.width 4
            , UiBorder.rounded 4
            , Ui.focused [ UiBorder.shadow { offset = ( 0, 0 ), size = 0, blur = 0, color = themeColor } ]
            , Ui.mouseDown [ UiBackground.color darkerThemeColor ]
            ]
    in
    { title = "OTA update"
    , body =
        [ Ui.layout [] <|
            Ui.row [ Ui.width Ui.fill, Ui.height Ui.fill, UiFont.color textColor ]
                [ Ui.column [ Ui.width Ui.fill, Ui.padding 24, Ui.spacing 40 ]
                    [ Ui.paragraph [ Ui.centerX, UiFont.center ]
                        [ Ui.text <|
                            case model.otaState of
                                OtaStateNone ->
                                    "Selezionare il firmware con cui aggiornare il dispositivo"

                                OtaStateRunning ->
                                    "Aggiornamento in corso..."

                                OtaStateSuccessful ->
                                    "Aggiornamento concluso con successo!"

                                OtaStateFailure ->
                                    "Aggiornamento fallito!"
                        ]
                    , Ui.column [ Ui.width Ui.fill, Ui.spacing 8, Ui.centerX ]
                        [ Ui.el [ Ui.centerX, UiFont.center ] <|
                            Ui.text filename
                        , Ui.row
                            [ Ui.width Ui.fill, Ui.spaceEvenly ]
                            [ UiInput.button
                                (buttonStyle ++ [ Ui.alignLeft ])
                                { onPress = Just FirmwareRequested, label = Ui.text "Seleziona" }
                            , Maybe.map
                                (\f ->
                                    UiInput.button
                                        (buttonStyle ++ [ Ui.alignRight ])
                                        { onPress = Just <| FirmwareUpdate f, label = Ui.text "Aggiorna" }
                                )
                                model.file
                                |> Maybe.withDefault Ui.none
                            ]
                        ]
                    ]
                    |> Ui.el
                        [ Ui.width (Ui.fill |> Ui.maximum 320)
                        , Ui.centerX
                        , Ui.centerY
                        , UiBackground.color darkColor
                        , UiBorder.color themeColor
                        , UiBorder.width 8
                        , UiBorder.rounded 16
                        ]
                ]
        ]
    }
