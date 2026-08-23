namespace temgi
{
    class ConsoleEventSubscriber {
        public:
            virtual ~ConsoleEventSubscriber() = default;

            virtual void onConsoleStart() {}
            virtual void onConsoleStop() {}

            virtual void onCartridgeLoad() {}
            virtual void onCartridgeUnload() {}

            virtual void onFrameStart() {}
            virtual void onFrameEnd() {}

    };
} // namespace temgi
